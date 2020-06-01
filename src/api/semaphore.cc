// EPOS Semaphore Implementation

#include <semaphore.h>
#include <process.h>

#include "instr_timer.h"

__BEGIN_SYS

Semaphore::Semaphore(int v): _value(v)
{
    db<Synchronizer>(TRC) << "Semaphore(value=" << _value << ") => " << this << endl;
}


Semaphore::~Semaphore()
{
    db<Synchronizer>(TRC) << "~Semaphore(this=" << this << ")" << endl;
}


void Semaphore::p()
{
    db<Synchronizer>(TRC) << "Semaphore::p(this=" << this << ",value=" << _value << ")" << endl;
    
    ITimer t;
    begin_atomic();

    if(fdec(_value) < 1)
    {    
        t.stop("sem_p_sleep", this);
        sleep(); // implicit end_atomic()
    }
    else
    {
        t.stop("sem_p_access", this);    
        end_atomic();
    }
}

void Semaphore::v()
{
    db<Synchronizer>(TRC) << "Semaphore::v(this=" << this << ",value=" << _value << ")" << endl;

    ITimer t;
    begin_atomic();

    if(finc(_value) < 0)
    {
        t.stop("sem_v_woke", this);
        wakeup();  // implicit end_atomic()
    }
    else
    {
        t.stop("sem_v_exit", this);
        end_atomic();
    }
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
  	  	t.stop("pcp_p_access", this);
  	  	priority(owner()->priority());
  	}
  	else
  	{
  	  	// There is an owner in my way
  	  	// I'll set its priority to the ceiling
  	  	t.stop("pcp_p_SetCeiling", this);
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

// MPCP IMPLEMENTATION
template<bool T>
void Semaphore_MPCP<T>::p()
{
	ITimer t;
	begin_atomic();
	if( !owner() )
	{
		owner( currentThread() );
		priority( owner()->priority() );
		if(T)
			owner()->priority( toGlobalCeiling() );
		else
			owner()->priority( ceiling() );
	}
	else { /* Couldn't get access, wait on synchronizer queue with normal_priority */ }

	t.stop("mpcp_p", this);
  	Semaphore_RT::p();
	kout << "Priority_t was " << priority() << ", now is " << owner()->priority() << endl;
  	end_atomic();
}

template<bool T>
void Semaphore_MPCP<T>::v()
{
	ITimer t;
  	begin_atomic();
	
  	if(owner() == currentThread())
  	{
		owner()->priority( priority() );
  	  	Thread * next = nextThread();

  	  	if(next)
  	  	{
  	  	  	owner(next);
  	  	  	priority( next->priority() );
			if(T)
				next->priority( toGlobalCeiling() );
			else
				next->priority( ceiling() );
  	  	}
  	  	else
  	  	  	owner(0);
  	}
	
  	t.stop("mpcp_v", this);
  	Semaphore_RT::v();
	kout << "Priority_t was " << priority() << ", now is " << owner()->priority() << endl;
  	end_atomic();
}

template void Semaphore_MPCP<true>::p();
template void Semaphore_MPCP<false>::p();
template void Semaphore_MPCP<true>::v();
template void Semaphore_MPCP<false>::v();

__END_SYS
