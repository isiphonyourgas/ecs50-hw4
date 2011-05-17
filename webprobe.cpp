// Aaron Okano, Jason Wong, Meenal Tambe, Gowtham Vijayaragavan
#include <iostream>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <cmath>
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
  int n = reinterpret_cast<int>( num );
  accesses[n] = 0;
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
    }
  }
  return 0;
}

// These two functions are for use by reporter
double findMean()
{
  double mean = 0;
  int i;
  for( i = 0; i < wwidth || recent[i] == -1; i++ )
  {
    mean += recent[i];
  }
  return mean / i;
}

double findStdDev()
{
  double stdDev = 0;
  double mean = findMean();
  double num;
  int i;
  for( i = 0; i < wwidth || recent[i] == -1; i++ )
  {
    num = recent[i];
    stdDev += ( num - mean) * ( num - mean) / i;
  }
  return sqrt(stdDev);
}


void *reporter( void *threads )
{
  // All this function needs to do is read the values in
  // accesses and recent and print them out.
  double length = 10.0/((int)threads) * 1000000;
  double sd, mean;
  int i;
  while(1)
  {
  usleep(length);
  pthread_mutex_lock( &mutex2 );
  mean = findMean();
  sd = findStdDev();
  cout << "\n\nRecent times:\n";
  cout << "Mean: " << mean << endl << "Standard Deviation: " << sd << endl;
  for(i = 0; i < (int)threads; i++)
    cout << "Threads " << i << ": " << accesses[i] << endl;
  pthread_mutex_unlock(&mutex2);

  }

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
    pthread_create( &id[i], NULL, &probe, reinterpret_cast<void*>(i) );
  // Create the reporting thread
  pthread_create( &id[numthreads-1], NULL, &reporter, (void*) numthreads );

  for( i = 0; i < numthreads; i++ )
    pthread_join( id[i], NULL );
}
