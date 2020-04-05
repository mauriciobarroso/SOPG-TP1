/*****************************inclusions *************************************/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>
/*****************************macros******************************************/
#define FIFO_NAME "myfifo"
#define BUFFER_SIZE 300
/*****************************data declaration********************************/
volatile sig_atomic_t signalNumber;
/*****************************functions declaration***************************/
void handler1( int sig );
void handler2( int sig );

/*****************************main********************************************/
int main()
{   
    /* data declaration */
    char outputBuffer[BUFFER_SIZE];
	uint32_t bytesWrote;
	int32_t returnCode, fd;
    struct sigaction sa;

    /* signal action: SIGUSR1 */
    sa.sa_handler = handler1;
    sa.sa_flags = 0;
    if( sigemptyset( &sa.sa_mask ) == -1)
    {
        perror( "sigemptyset" );
        exit( 1 );
    }
    if( sigaction( SIGUSR1, &sa, NULL ) == -1 )
    {
        perror( "sigaction" );
        exit( 1 );
    }

    /* signal action: SIGUSR2 */
    sa.sa_handler = handler2;
    sa.sa_flags = 0;
    if( sigemptyset( &sa.sa_mask ) == -1)
    {
        perror( "sigemptyset" );
        exit( 1 );
    }
    if( sigaction( SIGUSR2, &sa, NULL ) == -1 )
    {
        perror( "sigaction" );
        exit( 1 );
    }

    /* Create named fifo. -1 means already exists so no action if already exists */
    if ( ( returnCode = mknod( FIFO_NAME, S_IFIFO | 0666, 0 ) ) < -1 )
    {
        printf( "Error creating named fifo: %d\n", returnCode );
        exit( 1 );
    }   

    /* Open named fifo. Blocks until other process opens it */
	printf( "waiting for readers...\n" );
	if ( ( fd = open( FIFO_NAME, O_WRONLY ) ) < 0 )
    {
        printf( "Error opening named fifo file: %d\n", fd );
        exit( 1 );
    }

    /* open syscalls returned without error -> other process attached to named fifo */
	printf( "got a reader--type some stuff\n" );

    /* Loop forever */
	for( ;; )
	{
        /* Get some text from console */
		fgets( outputBuffer, BUFFER_SIZE, stdin );

        /* add headers */   
        char auxBuffer[ BUFFER_SIZE + sizeof( "DATA:" ) ];
        if( signalNumber != 0 )
        {
            sprintf( auxBuffer, "SIGN:%d\n", signalNumber);
            signalNumber = 0;
        }
        else   
            sprintf( auxBuffer, "DATA:%s", outputBuffer );
        
        /* Write buffer to named fifo. Strlen - 1 to avoid sending \n char */
		if (( bytesWrote = write( fd, auxBuffer, strlen( auxBuffer ) - 1 ) ) == -1 )
            perror( "write" );
        else
            printf( "writer: wrote %d bytes\n", bytesWrote );
	}
	return 0;
}
/*****************************functions definition****************************/
void handler1( int sig )
{
    signalNumber = 1;
}

void handler2( int sig )
{
    signalNumber = 2;
}