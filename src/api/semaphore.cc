// EPOS Semaphore Implementation

#include <synchronizer.h>
#include "inst-timer.h"

__BEGIN_SYS

Semaphore::Semaphore(int v): _value(v)
{
    db<Synchronizer>(TRC) << "Semaphore(value=" << _value << ") => " << this << endl;
}

Semaphore::~Semaphore()
{
    db<Synchronizer>(TRC) << "~Semaphore(this=" << this << ")" << endl;
}

void Semaphore::p(bool locked) {
	if(!locked)
    	begin_atomic();
	// ITimer t;

    if(fdec(_value) < 1) {
		// t.stop("sem_p", this);
        sleep(); // implicit end_atomic()
	}
    else {
		// t.stop("sem_p", this);
		if(!locked)
        	end_atomic();
	}
}

void Semaphore::v(bool locked) {
	if(!locked)
    	begin_atomic();
	// ITimer t;

    if(finc(_value) < 0) {
		// t.stop("sem_v", this);
        wakeup();  // implicit end_atomic()
	}
    else {
		// t.stop("sem_v", this);
		if(!locked)
        	end_atomic();
	}
}


void Semaphore_PIP::p() {
	begin_atomic();
	// ITimer t;

    if( !owner() ) {
        owner( currentThread() );
        priority( owner()->priority() );
		// t.stop("pip_p", this);
    }
    else {
		Thread_t * aux_current = currentThread();
        if( aux_current->priority() < owner()->priority() )
            owner()->setPriority( aux_current->priority() );
		// t.stop("pip_p", this);
    }

    Semaphore_RT::p(true);
    end_atomic();
}

void Semaphore_PIP::v() {
	begin_atomic();
	if(owner() == currentThread()) {
		// ITimer t;

		currentThread()->setPriority( priority() );
		Thread_t * next = nextThread();

		if(next) {
			owner( next );
			priority( owner()->priority() );
			// t.stop("pip_v", this);
		}
		else {
			owner(0);
			priority(0);
			// t.stop("pip_v", this);
		}
	}
	Semaphore_RT::v(true);
	end_atomic();
}

//Semaphore for Immediate Priority Ceiling Protocol
void Semaphore_IPCP::p() {
	begin_atomic();
	// ITimer t;

	if(!owner()) {
    	owner( currentThread() );
    	priority( owner()->priority() );
  	  	owner()->setPriority( ceiling() );
  	}

	// t.stop("ipcp_p", this);
  	Semaphore_RT::p(true);
  	end_atomic();
}

void Semaphore_IPCP::v() {
  	begin_atomic();

  	if(owner() == currentThread()) {
		// ITimer t;

		owner()->setPriority( priority() );
  	  	Thread_t * next = nextThread();

  	  	if(next) {
  	  	  	owner(next);
  	  	  	priority( owner()->priority() );
			owner()->setPriority( ceiling() );
			// t.stop("ipcp_v", this);
  	  	}
  	  	else {
  	  	  	owner(0);
			priority(0);
			// t.stop("ipcp_v", this);
		}
  	}
  	Semaphore_RT::v(true);
  	end_atomic();
}

//Semaphore for Priority Ceiling Protocol (classic)
void Semaphore_PCP::p() {
  	begin_atomic();
	// ITimer t;

  	if( !owner() ) {
  	  	owner( currentThread() );
  	  	priority( owner()->priority() );
		// t.stop("pcp_p", this);
  	}
  	else {
  	  	if(ceiling() < owner()->priority())
			owner()->setPriority( ceiling() );
		// t.stop("pcp_p", this);
  	}
  	Semaphore_RT::p(true);
  	end_atomic();
}

void Semaphore_PCP::v() {
  	begin_atomic();
	
  	if(owner() == currentThread()) {
		// ITimer t;

  	  	owner()->setPriority( priority() );
  	  	Thread_t * next = nextThread();

  	  	if(next) {
  	  	  	owner(next);

  	  	  	priority( owner()->priority() );
  	  	  	owner()->setPriority( ceiling() );
			// t.stop("pcp_v", this);
  	  	}
  	  	else {
  	  	  	owner(0);
			priority(0);
			// t.stop("pcp_v", this);
		}
  	}
  	Semaphore_RT::v(true);
  	end_atomic();
}

//Semaphore for Multicore Priority Ceiling Protocol
template<bool T>
void Semaphore_MPCP<T>::p() {
	begin_atomic();
	// ITimer t;

	if( !owner() ) {
		owner( currentThread() );
		priority( owner()->priority() );

		if(T)
			owner()->setPriority( toGlobalCeiling() );
		else
			owner()->setPriority( ceiling() );
	}

	// t.stop("mpcp_p", this);
  	Semaphore_RT::p(true);
  	end_atomic();
}

template<bool T>
void Semaphore_MPCP<T>::v() {
  	begin_atomic();
	
  	if(owner() == currentThread()) {
		// ITimer t;

		owner()->setPriority( priority() );
  	  	Thread * next = nextThread();

  	  	if(next) {
  	  	  	owner(next);
  	  	  	priority( next->priority() );

			if(T)
				owner()->setPriority( toGlobalCeiling() );
			else
				owner()->setPriority( ceiling() );
			// t.stop("mpcp_v", this);
  	  	}
  	  	else {
  	  	  	owner(0);
			priority(0);
			// t.stop("mpcp_v", this);
		}
  	}
  	Semaphore_RT::v(true);
  	end_atomic();
}

template void Semaphore_MPCP<true>::p();
template void Semaphore_MPCP<false>::p();
template void Semaphore_MPCP<true>::v();
template void Semaphore_MPCP<false>::v();

template<bool T>
void Semaphore_SRP<T>::p() {
	begin_atomic();
	Semaphore_RT<T>::p(true);

	ITimer t;
	updateCeiling();
	t.stop("srp_p", this);
	end_atomic();
}

template<bool T>
void Semaphore_SRP<T>::v() {
	begin_atomic();
	Semaphore_RT<T>::v(true);
	
	ITimer t;
	updateCeiling();
	t.stop("srp_v", this);

	end_atomic();
}

template void Semaphore_SRP<true>::p();
template void Semaphore_SRP<false>::p();
template void Semaphore_SRP<true>::v();
template void Semaphore_SRP<false>::v();

template<bool T>
Semaphore_SRP<T>* Semaphore_SRP<T>::_resources[MAX_RESOURCES];
template<bool T>
int Semaphore_SRP<T>::_nr = 0;
template<bool T>
int Semaphore_SRP<T>::_system_ceiling = Semaphore_SRP<T>::DEFAULT_CEILING;

bool Scheduling_Criteria::RT_Common::Elector_SRP::eligible(const Scheduling_Criteria::RT_Common * criterion) {
	/* Scheduler tries to choose idle or main threads */
	if( criterion->_priority == IDLE || criterion->_priority == MAIN )
		return true;
	
	/* Main or idle threads running */
	if( Thread::self()->criterion().preempt_level == 0 )
		return true; 
	
	return criterion->preempt_level > Semaphore_SRP<true>::systemCeiling();
}

bool Scheduling_Criteria::RT_Common::Elector_MSRP::eligible(const Scheduling_Criteria::RT_Common * criterion) {
	/* Scheduler tries to choose idle or main threads */
	ITimer t;
	if( criterion->_priority == IDLE || criterion->_priority == MAIN )
	{
		t.stop("eli", Thread::self());
		return true;
	}
	
	/* Main or idle threads running */
	if( Thread::self()->criterion().preempt_level == 0 )
	{
		t.stop("eli", Thread::self());
		return true; 
	}
	t.stop("eli", Thread::self());
	return (criterion->preempt_level > Semaphore_MSRP<true, false>::systemCeiling( criterion->queue() ) );
}

void Semaphore_MSRP<true, false>::p()
{
	begin_atomic();
	Semaphore_Template<false>::p(true);

	// ITimer t;
	updateCeiling();
	// t.stop("msrp_p", this);
	end_atomic();
}

void Semaphore_MSRP<true, false>::v()
{
	begin_atomic();
	Semaphore_Template<false>::v(true);

	// ITimer t;
	updateCeiling();
	// t.stop("msrp_v", this);
	end_atomic();
}

int Semaphore_MSRP<true, false>::_nGR = 0;
int Semaphore_MSRP<true, false>::_systemCeiling[_cpu];
Semaphore_MSRP<true, false>* Semaphore_MSRP<true, false>::_globalResources[MAX_RESOURCES];

__END_SYS
