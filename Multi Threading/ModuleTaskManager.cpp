#include "ModuleTaskManager.h"


void ModuleTaskManager::threadMain()
{
	// TODO 3:
	// - Wait for new tasks to arrive
	// - Retrieve a task from scheduledTasks
	// - Execute it
	// - Insert it into finishedTasks

	while(true){
		{
			std::unique_lock<std::mutex> lock(mtx);

			while (scheduledTasks.empty()) {

				event.wait(lock);

				//When exiting the application, get out of this infinite loop
				if (exitFlag)
					return;
			}

			Task* task_aux = scheduledTasks.front();
			scheduledTasks.pop();

			task_aux->execute();
			finishedTasks.push(task_aux);
			
		}
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
		Task* task_aux = finishedTasks.front();
		task_aux->owner->onTaskFinished(task_aux);
		finishedTasks.pop();

	}


	return true;
}

bool ModuleTaskManager::cleanUp()
{
	// TODO 5: Notify all threads to finish and join them
	{
		exitFlag = true;
		event.notify_all();
	}

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
	event.notify_one();

}
