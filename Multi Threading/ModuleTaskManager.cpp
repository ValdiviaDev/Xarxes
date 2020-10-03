#include "ModuleTaskManager.h"


void ModuleTaskManager::threadMain()
{
	std::unique_lock<std::mutex> lock(mtx);

	while (scheduledTasks.empty() && !exitFlag)
	{
		// TODO 3:
		// - Wait for new tasks to arrive
		// - Retrieve a task from scheduledTasks
		// - Execute it
		// - Insert it into finishedTasks
		
		event.wait(lock);


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

	return true;
}

bool ModuleTaskManager::cleanUp()
{
	// TODO 5: Notify all threads to finish and join them

	//booleano exitFlag hacer algo con esto para salir de los threads
	exitFlag = true;

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

}
