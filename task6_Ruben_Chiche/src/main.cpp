#include <iostream>
#include <thread>
#include <queue>
#include <tuple>
#include <mutex>

using namespace std;

// const number of producer
const size_t NUM_OF_PRODUCER = 2;

// the global queue
queue<tuple<thread::id, double, bool>> myContainer;

// mutex
mutex _mtx;

// consition variable
condition_variable cond_var;


// Producer
// Produce a Message into the queue
void produce() {

	thread::id myTid = this_thread::get_id();
	double myNum = hash<thread::id>{}(this_thread::get_id());

	for (myNum; myNum > 0; myNum /= 10)
	{
		{
			lock_guard lck_queue(_mtx);
			myContainer.push(tuple(myTid, myNum, false));
		}

		cond_var.notify_all();
	}

	{
		lock_guard lck_queue(_mtx);
		myContainer.push(tuple(myTid, myNum, true));
	}

	cond_var.notify_all();
}

// Consumer
// Consumer a Message from the queue
void consume() {

	int counter = NUM_OF_PRODUCER;

	while (counter)
	{
		std::tuple<thread::id, double, bool> element;

		{
			unique_lock lck(_mtx);
			cond_var.wait(lck, [&]() { return !myContainer.empty(); });
			element = myContainer.front();
			myContainer.pop();
		}

		if (std::get<2>(element)) {
			counter--;
			std::cout << std::get<0>(element) << " finished" << std::endl;
		}
		else {
			std::cout << std::get<0>(element) << " sent: " << std::get<1>(element) << std::endl;
		}
		
	}

}

int main()
{
	thread consumer(consume);
	thread producer1(produce);
	thread producer2(produce);

	producer1.join();
	producer2.join();
	consumer.join();

	return 0;
}
