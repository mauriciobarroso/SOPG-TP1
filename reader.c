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
/*****************************macros******************************************/
#define FIFO_NAME       "myfifo"
#define BUFFER_SIZE     300
#define MESSAGES_LIMIT  10
/*****************************main********************************************/
int main(void)
{
	uint8_t inputBuffer[ BUFFER_SIZE ];
	uint32_t bytesRead;
    int32_t returnCode, fd;
    FILE *fpLOG, *fpSIGNALS ;
    uint8_t messageCounter = 0;
    /* Create named fifo. -1 means already exists so no action if already exists */
    if ( ( returnCode = mknod(FIFO_NAME, S_IFIFO | 0666, 0 ) ) < -1  )
    {
        printf( "Error creating named fifo: %d\n", returnCode );
        exit( 1 );
    }
    
    /* Open named fifo. Blocks until other process opens it */
	printf( "waiting for writers...\n" );
	if ( ( fd = open( FIFO_NAME, O_RDONLY ) ) < 0 )
    {
        printf( "Error opening named fifo file: %d\n", fd );
        exit( 1 );
    }
    
    /* open syscalls returned without error -> other process attached to named fifo */
	printf( "got a writer\n" );
    /* creation of files log.txt and signals.txt */
    fpLOG = fopen ( "log.txt", "a" );
    if (fpLOG == NULL) 
    { 
        printf("Could not open Log.txt" ); 
        return 0; 
    } 

    fpSIGNALS = fopen ("signals.txt", "a" );
    if ( fpSIGNALS == NULL ) 
    { 
        printf( "Could not open Sign.txt" ); 
        return 0; 
    } 
    /* Loop until read syscall returns a value <= 0 */
	do
	{
        /* read data into local buffer */
		if ( ( bytesRead = read( fd, inputBuffer, BUFFER_SIZE ) ) == -1)
            perror( "read" );
        else
		{
			inputBuffer[ bytesRead ] = '\0';
			printf( "reader: read %d bytes: \"%s\"\n", bytesRead, inputBuffer );
		}
        /* write of files log.txt or signal.txt */
        if( !strncmp( "DATA:", inputBuffer, sizeof( 5 ) ) )
        {           
            fprintf( fpLOG, "%s\n", inputBuffer );
            printf( "reader: \"%s\" saved in log.txt\n", inputBuffer );
        }
        else
        {
            fprintf( fpSIGNALS, "%s\n", inputBuffer );
            printf( "reader: \"%s\" saved in signals.txt\n", inputBuffer );       
        }

        messageCounter++;
	}

	while ( bytesRead > 0 && messageCounter < MESSAGES_LIMIT );
    /* close of files log.txt and signals.txt */
    fclose( fpLOG ); 
    fclose( fpSIGNALS );

	return 0;
}