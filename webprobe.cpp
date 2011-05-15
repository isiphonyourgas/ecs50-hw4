// Aaron Okano, Jason Wong, Meenal Tambe, Gowtham Vijayaragavan
#include <iostream>
#include <pthread.h>
#include <fstream>
#include <cstdlib>
#include <ctime>
using namespace std;

// These will be arrays with a size to be defined later.
double *recent;
int *accesses;

ifstream urlfile;

// Probing function to be run by threads
void *probe( void *n)
{
  // Probing code goes here. The idea will be to time
  // how long it takes to run wget and then place the
  // running times in the recent array and the total
  // accesses to the accesses array.
  cout << "Probe\n";

}

void *reporter( void *threads )
{
  // All this function needs to do is read the values in
  // accesses and recent and print them out.
  cout << "Reporter\n";
}

void load_recent(int wwidth, double time){
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
  int wwidth = atoi(argv[2]);
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
  clock_t clo = clock(); 
  for( i = 0; i < numthreads-1; i++ )
    pthread_create( &id[i], NULL, &probe, (void*) i );
  // Create the reporting thread
  pthread_create( &id[numthreads-1], NULL, &reporter, (void*) numthreads );
  double timer = (clock() - clo)/(double)CLOCKS_PER_SEC;

  cout << "Time: " << timer << endl;

  for( i = 0; i < numthreads; i++ )
    pthread_join( id[i], NULL );
}
