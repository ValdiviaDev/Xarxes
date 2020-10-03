#include "ModuleTaskManager.h"


void ModuleTaskManager::threadMain()
{
	{ //Critical section
		std::unique_lock<std::mutex> lock(mtx);
		// TODO 3:
		// - Wait for new tasks to arrive
		// - Retrieve a task from scheduledTasks
		// - Execute it
		// - Insert it into finishedTasks
		while (!scheduledTasks.empty()) {
			event.wait(lock);
		}

		if (!scheduledTasks.empty()) {
			scheduledTasks.front()->execute();
			finishedTasks.push(scheduledTasks.front());
			scheduledTasks.pop();
		}

		
	}
}

bool ModuleTaskManager::init()
{
	// TODO 1: Create threads (they have to execute threadMain())
	for (int i = 0; i < MAX_THREADS; i++){
		threads[i] = std::thread(&ModuleTaskManager::threadMain, this); //sintaxis correcta?
	}

	return true;
}

bool ModuleTaskManager::update()
{
	// TODO 4: Dispatch all finished tasks to their owner module (use Module::onTaskFinished() callback)
	{ //Critical section
		std::unique_lock<std::mutex> lock(mtx);
		while (!finishedTasks.empty()) {
			onTaskFinished(finishedTasks.front());
			finishedTasks.pop();
		}
		event.notify_all();
	}

	return true;
}

bool ModuleTaskManager::cleanUp()
{
	// TODO 5: Notify all threads to finish and join them
	{
		std::unique_lock < std::mutex > lock(mtx);
		exitFlag = true;
		event.notify_all();

	}
	
	for (int i = 0; i < MAX_THREADS; i++) {
		threads[i].join();
	}

	return true;
}

void ModuleTaskManager::scheduleTask(Task *task, Module *owner)
{
	task->owner = owner;

	// TODO 2: Insert the task into scheduledTasks so it is executed by some thread
	scheduledTasks.push(task);
	//event.notify_all();

}
