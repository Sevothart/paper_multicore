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
	Base::p(true);
    //Semaphore_RT::p(true);
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
	Base::v(true);
	//Semaphore_RT<true, true>::v(true);
	end_atomic();
}

//Semaphore for Immediate Priority Ceiling Protocol
void Semaphore_IPCP::p() {
	begin_atomic();
	// ITimer t;

	if( !owner() ) {
    	owner( currentThread() );
		toCeiling();
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
  	  	  	toCeiling();
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
		toCeiling();
		// t.stop("pcp_p", this);
  	}
  	Semaphore_RT::p(true);
  	end_atomic();
}

// FIXME: Exatamente igual ao IPCP, is this wrong??
void Semaphore_PCP::v() {
  	begin_atomic();
	
  	if(owner() == currentThread()) {
		// ITimer t;

  	  	owner()->setPriority( priority() );
  	  	Thread_t * next = nextThread();

  	  	if(next) {
  	  	  	owner(next);
			priority( owner()->priority() );
			//toCeiling();

  	  	  	// priority( owner()->priority() );
  	  	  	// owner()->setPriority( ceiling() );
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
		toCeiling();
		// priority( owner()->priority() );
		kout << "Semaphore_MPCP<" << T << ">::p() -> owner priority: " << owner()->priority() << endl;
		//if(T)
			// owner()->setPriority( toGlobalCeiling() );
		// else
		// 	owner()->setPriority( ceiling() );
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
		kout << "Semaphore_MPCP<" << T << ">::v() -> restore owner priority: " << owner()->priority() << endl;

  	  	if(next) {
			kout << "Semaphore_MPCP<" << T << ">::v() -> there's a task waiting on the queue" << endl;
  	  	  	owner(next);
			toCeiling();
			kout << "Semaphore_MPCP<" << T << ">::v() -> owner priority: " << owner()->priority() << endl;
  	  	  	//priority( next->priority() );

			// if(T)
				//owner()->setPriority( toGlobalCeiling() );
			// else
			// 	owner()->setPriority( ceiling() );
			// t.stop("mpcp_v", this);
  	  	}
  	  	else {
			kout << "Semaphore_MPCP<" << T << ">::v() -> no more tasks waiting on the queue" << endl;
  	  	  	owner(0);
			priority(0);
			// t.stop("mpcp_v", this);
		}
  	}
  	Semaphore_RT::v(true);
  	end_atomic();
}

template void Semaphore_MPCP<true>::p();
template void Semaphore_MPCP<true>::v();

template<bool T>
void Semaphore_SRP<T>::p() {
	begin_atomic();
	Base::p(true);
	//Semaphore_RT<true, true>::p(true);

	//ITimer t;
	updateCeiling();
	//t.stop("srp_p", this);
	end_atomic();
}

template<bool T>
void Semaphore_SRP<T>::v() {
	begin_atomic();
	Base::v(true);
	//Semaphore_RT<true, true>::v(true);
	
	//ITimer t;
	updateCeiling();
	//t.stop("srp_v", this);
	end_atomic();
}

template void Semaphore_SRP<true>::p();
template void Semaphore_SRP<false>::p();
template void Semaphore_SRP<true>::v();
template void Semaphore_SRP<false>::v();

template<bool T> Semaphore_SRP<T>* Semaphore_SRP<T>::_resources[MAX_RESOURCES];
template Semaphore_SRP<true>* Semaphore_SRP<true>::_resources[MAX_RESOURCES];
template Semaphore_SRP<false>* Semaphore_SRP<false>::_resources[MAX_RESOURCES];

template<bool T> int Semaphore_SRP<T>::_nr = 0;
template int Semaphore_SRP<true>::_nr;
template int Semaphore_SRP<false>::_nr;

template<bool T> int Semaphore_SRP<T>::_system_ceiling = Semaphore_SRP<T>::DEFAULT_CEILING;
template int Semaphore_SRP<true>::_system_ceiling;
template int Semaphore_SRP<false>::_system_ceiling;

bool Scheduling_Criteria::RT_Common::Elector_SRP::eligible(const Scheduling_Criteria::RT_Common * criterion) {
	/* Scheduler tries to choose idle or main threads */
	if( criterion->_priority == IDLE || criterion->_priority == MAIN )
		return true;
	
	/* Main or idle threads running */
	if( Thread::self()->preemptLevel() == 0 )
		return true; 
	
	return criterion->preempt_level > Semaphore_SRP<true>::systemCeiling();
}

bool Scheduling_Criteria::RT_Common::Elector_MSRP::eligible(const Scheduling_Criteria::RT_Common * criterion) {
	/* Scheduler tries to choose idle or main threads */
	if( criterion->_priority == IDLE || criterion->_priority == MAIN )
		return true;
	
	/* Main or idle threads running */
	if( Thread::self()->preemptLevel() == 0 )
		return true; 

	kout << "Elector_MSRP::eligible(): preemptLevel: " << criterion->preempt_level << " < systemCeiling("
		<< criterion->queue() << "): " << Semaphore_MSRP::systemCeiling( criterion->queue() ) << endl;
	return (criterion->preempt_level > Semaphore_MSRP::systemCeiling( criterion->queue() ) );
}

void Semaphore_MSRP::p()
{
	begin_atomic();
	Semaphore_RT<false, true>::p(true);
	//Semaphore_Template<false>::p(true);

	// ITimer t;
	updateCeiling();
	// t.stop("msrp_p", this);
	end_atomic();
}

void Semaphore_MSRP::v()
{
	begin_atomic();
	Semaphore_RT<false, true>::v(true);
	//Semaphore_Template<false>::v(true);

	// ITimer t;
	updateCeiling();
	// t.stop("msrp_v", this);
	end_atomic();
}

int Semaphore_MSRP::_nGR = 0;
int Semaphore_MSRP::_systemCeiling[_cpu];
Semaphore_MSRP * Semaphore_MSRP::_globalResources[MAX_RESOURCES];

__END_SYS
