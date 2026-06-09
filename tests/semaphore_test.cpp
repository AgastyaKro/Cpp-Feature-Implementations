#include "../include/BlockingCountingMutexSemaphore.hpp"

#include <cassert>

void test_basic_try_acquire(){
    BlockingCountingMutexSemaphore sem(1);

    assert(sem.try_acquire() == true);
    assert(sem.try_acquire() == false);

    sem.release();
    
    assert(sem.try_acquire() == true);

}

void test_acquire_then_release(){
    BlockingCountingMutexSemaphore sem(1);

    sem.acquire();

    assert(sem.try_acquire() == false);

    sem.release();

    assert(sem.try_acquire() == true);

}

void test_guard_auto_release(){
    BlockingCountingMutexSemaphore sem(1);

    {
        auto permit = sem.acquire_guard();
        assert(sem.try_acquire() == false);
    }

    assert(sem.try_acquire() == true);
}

void test_try_acquire_guard_sucess(){
    BlockingCountingMutexSemaphore sem(1);
    auto permit = sem.try_acquire_guard();

    assert(permit.has_value());
    assert(sem.try_acquire() == false);
}





