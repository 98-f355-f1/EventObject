#include <Windows.h>
#include <stdio.h>

#define THREADCOUNT 4

HANDLE ghWriteEvent;
HANDLE ghThreads[THREADCOUNT];

DWORD WINAPI ThreadProc(LPVOID);

void CreateEventsAndThreads(void)
{
	int i;
	DWORD dwThreadID;

	// Create a manual-reset event object.
	// The write thread sets this object
	// to the signaled state when it finishes
	// writing to a shared buffer
	ghWriteEvent = CreateEvent(
		NULL,		// default security attributes
		TRUE,		// manual-result event
		FALSE,		// initial state is nonsignaled
		TEXT("Tom Branson"));

	if (ghWriteEvent == NULL)
	{
		printf("CreateEvent failed (%d)\n", GetLastError());
		return;
	}

	// Create multiple threads to read from the buffer
	for (i = 0; i < THREADCOUNT; i++)
	{
		// TODO: more complex scenarios may require use
		// of a parameter to the thread procedure, such
		// as an event per thread to be used for synchronization
		ghThreads[i] = CreateThread(
			NULL,		// default security
			0,			// default stack size
			ThreadProc,	// name of the thread function
			(LPVOID)i,	// thread arguments
			0,			// no thread parameters
			&dwThreadID);
		
		if (ghThreads[i] == NULL)
		{
			printf("CreateThread failed (%d)\n", GetLastError());
			return;
		}
	}
}

void WriteToBuffer(VOID)
{
	// TODO: Write to the shared buffer
	printf("Main thread writing to the shared buffer....\n");

	// Set ghWriteEvent to signaled
	if (!SetEvent(ghWriteEvent))
	{
		printf("SetEvent failed (%d)", GetLastError());
		return;
	}
}

void CloseEvents()
{
	// Close all event handles (currently, only one global handle).
	CloseHandle(ghWriteEvent);
}

int main(void)
{
	DWORD dwWaitResult;

	//TODO: create the shared buffer

	// Create events and THREADCOUNT threads to
	// read from the buffer.
	CreateEventsAndThreads();

	// At this point the reader threads have started
	// and are most likey waiting for the global event
	// to be signaled.  However, it is safe to write
	// to the buffer because the event is a manual-reset
	// event.

	WriteToBuffer();

	printf("Main thread waiting for threads to exit...\n");

	// The handle for each thread is signaled when the
	// thread is terminated.
	dwWaitResult = WaitForMultipleObjects(
		THREADCOUNT,		// number of handles in array
		ghThreads,			// array of thread handles
		TRUE,				// wait until all are signaled
		INFINITE);

	switch(dwWaitResult)
	{
		// All thread objects were signaled
		case WAIT_OBJECT_0:
			printf("All threads ended, cleaning up for application exit...\n");
			break;
		default:
			printf("WaitForMultipleObjects failed (%d)\n", GetLastError());
			return 1;
	}

	// Close the events to clean up
	CloseEvents();
	getchar();

	return 0;
}

DWORD WINAPI ThreadProc(LPVOID lpParam)
{
	// if lpParam not used
	// UNREFERENCED_PARAMETER(lpParam)
	UINT64 lpP = (UINT64)(lpParam);

	DWORD dwWaitResult;

	printf("Thread ID: %d #%d waiting for write event...\n", GetCurrentThreadId(), lpP);

	dwWaitResult = WaitForSingleObject(
		ghWriteEvent,	// event handle
		INFINITE);		// indefinite wait

	switch (dwWaitResult)
	{
		// Event object was signaled
		case WAIT_OBJECT_0:
			// TODO: read from the shared buffer
			printf("Thread %d reading from buffer\n", GetCurrentThreadId());
			break;
		default:
			printf("Wait error (%d)\n", GetLastError());
			return 0;
	}

	// Now that we are done reading the buffer, we could use
	// another event signal this thread is no longer reading.
	// This example uses the thread handle for synchronization
	// (the handle is signaled when the thread terminates.)
	printf("Thread %d exiting\n", GetCurrentThreadId());
	return 1;
}
