// Aaron Okano, Jason Wong, Meenal Tambe, Gowtham Vijayaragavan
#include <iostream>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fstream>
#include <string>
#include <cstdlib>
#include <ctime>
using namespace std;

// These will be arrays with a size to be defined later.
double *recent;
int *accesses;

int wwidth;

// Various mutexes declared here
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

ifstream urlfile;

// Store the wwidth most recent run times
void load_recent(double time){
  for (int i = 0; i < wwidth; i++){
    if (recent[wwidth] == -1){
      break; 
  }
  // if i == wwidth, then array recent is full. Have to overwrite. 
  if (i == wwidth){
    for (int j = 0; j < wwidth-1; j++){
      recent[j] = recent[j + 1];  
    }
  recent[wwidth] = time; 
  }
  //else iterator i stopped at a empty array position. 
  //Fill that position with the time to input. 
  else recent[i] = time;      
  }
}

// Probing function to be run by threads
void *probe( void *num)
{
  // Probing code goes here. The idea will be to time
  // how long it takes to run wget and then place the
  // running times in the recent array and the total
  // accesses to the accesses array.
  int n = (int) num;
  string url;
  pid_t pID;
  int stat;
  struct timeval time1, time2;
  double timer;
  while(1)
  {
    pthread_mutex_lock( &mutex1 );
    getline( urlfile, url );
    pthread_mutex_unlock( &mutex1 );
    gettimeofday( &time1, 0 );
    pID = fork();
    if( pID == 0 )
    {
      execlp("wget","wget","--spider","-q",url.c_str(),NULL);
    }
    else
    {
      waitpid( pID, &stat, 0 );
    }
    gettimeofday( &time2, 0 );
    if( stat == 0 )
    {
      // Complicated mess of crap to subtract ending time from starting time
      timer = 1000000 * (time2.tv_sec - time1.tv_sec) + 
          (time2.tv_usec - time1.tv_usec);
      timer /= 1000000;
      // load_time() makes changes to a couple shared variables,
      // so we lock this section first
      pthread_mutex_lock( &mutex2 );
      load_recent(timer);
      // Increment the count for number of accesses
      accesses[n]++;
      pthread_mutex_unlock( &mutex2 );
      cout << "Probe time: " << timer << endl;
    }
  }
  return 0;
}

void *reporter( void *threads )
{
  // All this function needs to do is read the values in
  // accesses and recent and print them out.
  cout << "Reporter\n";
}

int main( int argc, char *argv[] )
{
  // Check if the number of arguments lines up.
  if( argc != 4 )
  {
    cout << "Too few arguments" << endl;
    cout << "Usage: webprobe urlfile wwidth numthreads" << endl;
    return 0;
  }
  // Start setting all the variables
  urlfile.open(argv[1], ios::in);
  wwidth = atoi(argv[2]);
  int numthreads = atoi(argv[3]);

  // Now set up the arrays
  pthread_t *id = new pthread_t[numthreads];
  accesses = new int[numthreads];
  recent = new double[wwidth];
  for( int i = 0; i < wwidth; i++){
    recent[i] = -1; 
  }
  // Create the probe threads
  int i;
  for( i = 0; i < numthreads-1; i++ )
    pthread_create( &id[i], NULL, &probe, (void*) i );
  // Create the reporting thread
  pthread_create( &id[numthreads-1], NULL, &reporter, (void*) numthreads );

  for( i = 0; i < numthreads; i++ )
    pthread_join( id[i], NULL );
}
