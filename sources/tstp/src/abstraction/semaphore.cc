// EPOS Semaphore Abstraction Implementation

#include <semaphore.h>
#include "thread.h"
#include <tracer.h>
#include "instr_timer.h"

__BEGIN_SYS

Semaphore::Semaphore(int v): _value(v)
{
    db<Synchronizer>(TRC) << "Semaphore(value=" << _value << ") => " << this << endl;
    
    if(traced) Tracer::semaphoreCreated(this, v);
}


Semaphore::~Semaphore()
{
    db<Synchronizer>(TRC) << "~Semaphore(this=" << this << ")" << endl;
}


void Semaphore::p()
{
    db<Synchronizer>(TRC) << "Semaphore::p(this=" << this << ",value=" << _value << ")" << endl;
    
    ITimer t;
    int nv;

    begin_atomic();
    if(fdec(_value) < 1)
    {
        if(traced) nv =  _value;       
        t.stop("sem_p", this);
        sleep(); // implicit end_atomic()
    }
    else
    {
        if(traced) nv = _value;    
        t.stop("sem_p", this);    
        end_atomic();
        
    }
    
    if(traced)Tracer::semaphoreChanged(this, nv);
}


void Semaphore::v()
{
    db<Synchronizer>(TRC) << "Semaphore::v(this=" << this << ",value=" << _value << ")" << endl;

    ITimer t;
    
    begin_atomic();
    if(finc(_value) < 0)
    {
        t.stop("sem_v", this);
        wakeup();  // implicit end_atomic()
    }
    else
    {
        t.stop("sem_v", this);
        end_atomic();
    }
}


void Semaphore_PIP::p()
{
    ITimer t;
    begin_atomic();
    if(owner()==0)
    {
        owner(currentThread());
        priority(owner()->priority());
    }
    else
    {
        Thread * aux_running;
        aux_running = reinterpret_cast< Thread_t *>(Thread::self());
        if(aux_running->priority() < owner()->priority()){
            owner()->priority(Thread_t::Priority(aux_running->priority()));         
        }
    }
    t.stop("pip_p", this);
    Semaphore_RT::p();
    end_atomic();
}

void Semaphore_PIP::v()
{
  begin_atomic();
  ITimer t;
  if(owner()!=0)    
  {
      
      Thread_t * aux_running;
      aux_running = reinterpret_cast< Thread_t *>(Thread::self());
      t.stop("pip_v_outer", this);
      aux_running->priority(priority());
      ITimer t2;
      if(!_queue.empty())
      {
          owner(nextThread());
          priority(owner()->priority());
      }
      else
      {
          owner(0);
          priority(0);
      }
      t2.stop("pip_v_inner", this);
  }
  else
  t.stop("pip_v_outer", this);

  Semaphore_RT::v();
  end_atomic();
}


// IPCP IMPLEMENTATION

void Semaphore_IPCP::p()
{
    ITimer t;
  begin_atomic();
    
  if(!owner())
  {
    // I am the new owner
    owner(currentThread());
    priority(owner()->priority());
    owner()->priority(ceiling());
  }
  else
  {
    // I will wait in the queue
    // Nothing to be done for me
  }
  
  t.stop("ipcp_p", this);
  Semaphore_RT::p();
  end_atomic();
}

void Semaphore_IPCP::v()
{
    ITimer t;
  begin_atomic();
  
  if(owner() == currentThread()) //this should be true, otherwise some kind of "double free" happened
  {
    owner()->priority(priority());
      
    Thread_t * next = nextThread();
    
    if(next)
    {
      // the next thread is the new owner
      owner(next);
      
      //immediately set the new priority
      priority(next->priority());
      next->priority(ceiling());
      
    }
    else
    {
      owner(0); // nobody owns this semaphore anymore
    }
  }
  
  t.stop("ipcp_v", this);
  Semaphore_RT::v();
  end_atomic();
}



// PCP IMPLEMENTATION

void Semaphore_PCP::p()
{
  begin_atomic();
    ITimer t;
    
  if(!owner())
  {
    // I am the new owner
    owner(currentThread());
    t.stop("pcp_p", this);
    priority(owner()->priority());
  }
  else
  {
    // There is an owner in my way
    // I'll set its priority to the ceiling
    t.stop("pcp_p", this);
    if(owner()->priority() < ceiling()) owner()->priority(ceiling());
  }
  
  Semaphore_RT::p();
  end_atomic();
}

void Semaphore_PCP::v()
{
  begin_atomic();
  
  if(owner() == currentThread()) //this should be true, otherwise some kind of "double free" happened
  {
    owner()->priority(priority()); // restore the priority of this owner thread
    ITimer t;
      
    Thread_t * next = nextThread();
    
    if(next)
    {
      // the next thread is the new owner
      owner(next);
      
      // immediately set the new priority 
      priority(next->priority());
      next->priority(ceiling());
    }
    else
    {
      owner(0); // nobody owns this semaphore anymore
    }
    t.stop("pcp_v", this);
  }
  
  Semaphore_RT::v();
  end_atomic();
}



// SRP IMPLEMENTATION

void Semaphore_SRP::p()
{
  Semaphore_RT::p();
  begin_atomic();
    ITimer t;
  updateCeiling();
  t.stop("srp_p", this);
  end_atomic();
}

void Semaphore_SRP::v()
{
  Semaphore_RT::v();
  begin_atomic();
    ITimer t;
  updateCeiling();
  t.stop("srp_v", this);
  end_atomic();
}

Semaphore_SRP* Semaphore_SRP::_resources[MAX_RESOURCES];
int Semaphore_SRP::_nr = 0;
int Semaphore_SRP::_system_ceiling = Semaphore_SRP::DEFAULT_CEILING;

bool Scheduling_Criteria::RT_Common::Elector_SRP::eligible(const Scheduling_Criteria::RT_Common * criterion)
{
    
    int prio = ((int)(*criterion));
    
//     kout << "SRP eligible? ";
    
    if(prio == IDLE || prio == MAIN)
    {
//         kout << "YES" << endl;
        return true; // these threads always eligible
    }
    
//     kout << (-(int)_deadline) << " : " << Semaphore_SRP::systemCeiling() << endl;
    
    bool el = (-(int)criterion->_deadline) > Semaphore_SRP::systemCeiling();
//     kout << (el?"YES":"NO") << endl;
    
    return el;
}

__END_SYS
