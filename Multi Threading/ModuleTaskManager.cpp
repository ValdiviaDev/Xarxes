#include "ModuleTaskManager.h"


void ModuleTaskManager::threadMain()
{

	// TODO 3:
	// - Wait for new tasks to arrive
	// - Retrieve a task from scheduledTasks
	// - Execute it
	// - Insert it into finishedTasks

	{
		std::unique_lock<std::mutex> lock(mtx);

		while (scheduledTasks.empty()) {

			event.wait(lock);
		}

		Task* task_aux = scheduledTasks.front();

		scheduledTasks.front()->execute();
		scheduledTasks.pop();
		finishedTasks.push(task_aux);

		
	}
}

bool ModuleTaskManager::init()
{
	// TODO 1: Create threads (they have to execute threadMain())
	for (int i = 0; i < MAX_THREADS; i++) {
		threads[i] = std::thread(&ModuleTaskManager::threadMain, this);
	}

	return true;
}

bool ModuleTaskManager::update()
{
	// TODO 4: Dispatch all finished tasks to their owner module (use Module::onTaskFinished() callback)

	while (!finishedTasks.empty()) {
		Module::onTaskFinished(finishedTasks.front());
		finishedTasks.pop();
		//join?
	}
	if (!scheduledTasks.empty())
		event.notify_one();


	return true;
}

bool ModuleTaskManager::cleanUp()
{
	// TODO 5: Notify all threads to finish and join them
	{
		std::unique_lock < std::mutex > lock(mtx);
		exitFlag = true;
		event.notify_all();


	for (int i = 0; i < MAX_THREADS; i++) {
		threads[i].join();
	}

	return true;
}

void ModuleTaskManager::scheduleTask(Task* task, Module* owner)
{
	task->owner = owner;

	// TODO 2: Insert the task into scheduledTasks so it is executed by some thread
	scheduledTasks.push(task);
	//event.notify_all();

	event.notify_one(); //?¿?¿

}
