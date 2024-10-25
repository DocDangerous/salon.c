//Caleb Harris, ID 5439602, P6

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct customer {
    char name[21];
    char prefstylist[21];
    int arrival;
    int loyaltypts;
    int haircutlength;
} customer;

typedef struct heap {
    char stylist[21];
    customer** list;
    int n;
    int maxn;
    int curtime;
    int waiting;
} heap;

typedef struct report {
    char cusname[21];
    char stylname[21];
    int finishtime;
    int finloyalty;
    int stylnum;
} report;

typedef struct reportholder {
    report** finishlist;
    int size;
} reportholder;

//Heap functions
void init(heap* hPtr);
void deleteMin(heap* hPtr);
void insert(heap* hPtr, customer* item, char stylist[]);
int compare(customer* a, customer* b, char stylist[]);
void swap(customer** ptrA, customer** ptrB);
void percolateUp(heap* hPtr, int index, char stylist[]);
void percolateDown(heap* hPtr, int index, char stylist[]);

//Salon functions
void newArrival(customer* newbie, heap* stylists, int m);
void update(int newtime, heap* stylists, int m, reportholder* repholder);
void finalUpdate(heap* stylists, int m, reportholder* repholder);
void printResults(report** array, int n);

//Sort functions
void quicksort(report** array, int low, int high);
void swap2(report* a, report* b);
void insertionSort(report** array, int low, int high);
int randMedian(report** array, int low, int high);
int partition(report** vals, int low, int high);
int is_sorted(report** array, int low, int high);

//Main
int main()
{
    srand(time(0));

    //Get number of customers
    int n;
    scanf("%d", &n);
    //Get number of stylists
    int m;
    scanf("%d", &m);

    //Statically allocate array of heaps representing lines for each stylist.
    heap stylists[10];

    //Assign name of each stylist and initialize heaps
    for(int i = 0; i < m; i++) {
        scanf("%s", stylists[i].stylist);
        init(&stylists[i]);
        stylists[i].curtime = 0;
    }


    //Get info for all of the day's customers and store in an array
    customer* allCustomers = (customer*)malloc(n * sizeof(customer));
    for(int i = 0; i < n; i++) {
        scanf("%d", &allCustomers[i].arrival);
        scanf("%s", allCustomers[i].name);
        scanf("%s", allCustomers[i].prefstylist);
        scanf("%d", &allCustomers[i].loyaltypts);
        scanf("%d", &allCustomers[i].haircutlength);
    }

    //Make array to store finished haircut data to output at end
    reportholder repholder;
    repholder.finishlist = malloc(n * sizeof(report *));
    for(int i = 0; i < n; i++) {
        repholder.finishlist[i] = malloc(sizeof(report));
    }
    //This "size" tracks the current index to store data at
    repholder.size = 0;

    //Process each arrival at the salon
    for(int i = 0; i < n; i++) {
        update(allCustomers[i].arrival, stylists, m, &repholder);
        newArrival(&allCustomers[i], stylists, m);
    }

    //Evaluate until all customers are out of queue
    finalUpdate(stylists, m, &repholder);

    //Free heaps
    for(int i = 0; i < m; i++) {
        free(stylists[i].list);
    }

    //Free customer list
    free(allCustomers);

    //sort output array
    quicksort(repholder.finishlist, 0, n-1);

    //Output
    printResults(repholder.finishlist, n);

    //Free report
    for(int i = 0; i < n; i++) {
        free(repholder.finishlist[i]);
    }
    free(repholder.finishlist);
}

// Initializes the heap pointed to by hPtr.
void init(heap* hPtr) {

    // Make enough space - make each pointer NULL.
    hPtr->list = malloc(sizeof(customer*)*(2));
    int i;
    for (i=0; i<2; i++) hPtr->list[i] = NULL;
    hPtr->n = 0;
    hPtr->maxn = 1;
    hPtr->waiting = 0;
}

//Process a new arrival. Figure out which stylist they go to and insert them into their queue.
void newArrival(customer* newbie, heap* stylists, int m) {
    //Go through each stylist. If that's the customer's preferred stylist, put them in that line. If not, determine who has the shortest line.
    int shortestline = 0;

    for(int i = 0; i < m; i++) {
        if(strcmp(newbie->prefstylist, stylists[i].stylist) == 0) {
            insert(&stylists[i], newbie, stylists[i].stylist);
            return;
        }
        if(stylists[i].waiting < stylists[shortestline].waiting) {
            shortestline = i;
        }
    }

    //Default to put customer in shortest line
    insert(&stylists[shortestline], newbie, stylists[shortestline].stylist);
    return;
}

//Progress the salon time from the end of the last update, to the time of the latest arrival
void update(int newtime, heap* stylists, int m, reportholder* repholder) {
    //Go through each stylist's queue while time is behind time of latest arrival
    for(int i = 0; i < m; i++) {
        while(stylists[i].curtime < newtime) {
            //If queue is empty, sync stylist's local time with current time and break
            if(stylists[i].n == 0) {
                stylists[i].curtime = newtime;
                break;
            }
            else {
                //Adjust stylist's "current time," the "real time" of the day in which they'd be after finishing with a client, add loyalty points, then add them to the "finished" array
                //Get new current time
                stylists[i].curtime += stylists[i].list[1]->haircutlength;
                //Calculate loyalty points
                repholder->finishlist[repholder->size]->finloyalty = stylists[i].list[1]->loyaltypts + (stylists[i].list[1]->haircutlength / 10);
                //Get finish time
                repholder->finishlist[repholder->size]->finishtime = stylists[i].curtime;
                //Get stylist name
                strcpy(repholder->finishlist[repholder->size]->stylname, stylists[i].stylist);
                //Get stylist num
                repholder->finishlist[repholder->size]->stylnum = i;
                //Get customer name
                strcpy(repholder->finishlist[repholder->size]->cusname, stylists[i].list[1]->name);
                //Increase final report index
                repholder->size++;
                //Decrease number of people in line
                stylists[i].waiting--;
                //Remove from queue
                deleteMin(&stylists[i]);
                //If someone is actively getting a haircut during current time, act as if someone were in line for sake of new arrival function
                if(stylists[i].curtime > newtime) {
                    stylists[i].waiting++;
                }
            }
        }
    }
}

//Empty the hair salon. Final update that ends once queues are empty.
void finalUpdate(heap* stylists, int m, reportholder* repholder) {
    //Go through each stylist's queue while time is behind time of latest arrival
    for(int i = 0; i < m; i++) {
        while(stylists[i].n != 0) {
                //Adjust stylist's "current time," the "real time" of the day in which they'd be after finishing with a client, add loyalty points, then add them to the "finished" array
                //Get new current time
                stylists[i].curtime += stylists[i].list[1]->haircutlength;
                //Calculate loyalty points
                repholder->finishlist[repholder->size]->finloyalty = stylists[i].list[1]->loyaltypts + (stylists[i].list[1]->haircutlength / 10);
                //Get finish time
                repholder->finishlist[repholder->size]->finishtime = stylists[i].curtime;
                //Get stylist name
                strcpy(repholder->finishlist[repholder->size]->stylname, stylists[i].stylist);
                //Get stylist num
                repholder->finishlist[repholder->size]->stylnum = i;
                //Get customer name
                strcpy(repholder->finishlist[repholder->size]->cusname, stylists[i].list[1]->name);
                //Increase final report index
                repholder->size++;
                //Decrease number of people in line
                stylists[i].waiting--;
                //Remove from queue
                deleteMin(&stylists[i]);
        }
    }
}

// Returns a negative integer if a < b, a positive integer if a > b, 0 if a == b.
int compare(customer* a, customer* b, char stylist[]) {
    if(a->loyaltypts == b->loyaltypts) {
        if((strcmp(a->prefstylist, stylist) == 0) && (strcmp(b->prefstylist, stylist) == 0)) {
            return strcmp(a->name, b->name);
        }
        else {
            if(strcmp(a->prefstylist, stylist) == 0) {
                return -1;
            }
            else {
                if (strcmp(b->prefstylist, stylist) == 0) {
                    return 1;
                }
                else {
                    return strcmp(a->name, b->name);
                }
            }
        }
    }
    else {
        if(a->loyaltypts > b->loyaltypts) {
            return -1;
        }
        else {
            return 1;
        }
    }
}

void percolateUp(heap* hPtr, int index, char stylist[]) {

    // At root, can't go up any more.
    if (index == 1) return;

    int pIndex = index/2;

    // Node and parent are out of order, swap and recurse.
    if (compare(hPtr->list[index], hPtr->list[pIndex], stylist) < 0) {
        swap(&hPtr->list[index], &hPtr->list[pIndex]);
        percolateUp(hPtr, pIndex, stylist);
    }
}

void percolateDown(heap* hPtr, int index, char stylist[]) {

    // Leaf node.
    if (2*index > hPtr->n) return;

    // You only have a left child.
    if (2*index == hPtr->n) {

        // Last swap.
        if (compare(hPtr->list[2*index], hPtr->list[index], stylist) < 0)
            swap(&hPtr->list[2*index], &hPtr->list[index]);
        return;
    }

    // Figure out whether or not the left or right child is better.
    int swapIndex = compare(hPtr->list[2*index], hPtr->list[2*index+1], stylist) < 0 ? 2*index : 2*index+1;

    // Node and parent are out of order, swap and recurse.
    if (compare(hPtr->list[swapIndex], hPtr->list[index], stylist) < 0) {
        swap(&hPtr->list[swapIndex], &hPtr->list[index]);
        percolateDown(hPtr, swapIndex, stylist);
    }
}

void insert(heap* hPtr, customer* item, char stylist[]) {

    // Increment the size of the heap. Realloc if heap needs to be expanded
    hPtr->n++;
    if(hPtr->n == hPtr->maxn) {
        hPtr->maxn = hPtr->maxn * 2;
        hPtr->list = realloc(hPtr->list, hPtr->maxn * sizeof(customer*));
    }

    //Increment tracker for number of people in line
    hPtr->waiting++;

    // Copy new item into first open slot in the heap.
    hPtr->list[hPtr->n] = item;

    // Now percolate this item up so it goes to its rightful place.
    percolateUp(hPtr, hPtr->n, stylist);
}

// Pre-condition - heap must be non-empty, otherwise this crashes.
void deleteMin(heap* hPtr) {

    // Copy last item into first slot.
    hPtr->list[1] = hPtr->list[hPtr->n];

    // Now our heap has one fewer item.
    hPtr->n--;

    // Now, we just have to percolate this item down.
    percolateDown(hPtr, 1, hPtr->stylist);
}

// Swaps the pointers ptrA and ptrB, so that the pointers in the array that they come from
// have changed what they point to.
void swap(customer** ptrA, customer** ptrB) {
    customer* tmp = *ptrA;
    *ptrA = *ptrB;
    *ptrB = tmp;
}

//Function to swap report pointers
void swap2(report* a, report* b) {

     report temp = *a;
     *a = *b;
     *b = temp;
}

//Function to pull a median of three random indices for use as partition
int randMedian(report** array, int low, int high) {

    //Generate random indices
    int r1, r2, r3;
    r1 = low + rand()%(high-low+1);
    r2 = low + rand()%(high-low+1);
    r3 = low + rand()%(high-low+1);

    //Check to find median value in each corresponding index
    if((array[r1]->finishtime <= array[r2]->finishtime && array[r1]->finishtime >= array[r3]->finishtime) || (array[r1]->finishtime <= array[r3]->finishtime && array[r1]->finishtime >= array[r2]->finishtime)) {
        return r1;
    }
    else {
        if((array[r2]->finishtime <= array[r1]->finishtime && array[r2]->finishtime >= array[r3]->finishtime) || (array[r2]->finishtime <= array[r3]->finishtime && array[r2]->finishtime >= array[r1]->finishtime)) {
            return r2;
        }
        else {
            return r3;
        }
    }
}

//Function to find/use partition to sort array
int partition(report** vals, int low, int high) {

    int temp;
    int i, lowpos;

    //Call function to find median value of random indices and set it as new low
    i = randMedian(vals, low, high);
    swap2(vals[low], vals[i]);

	//Store the index of the partition element.
	lowpos = low;

	//Update the low pointer.
	low++;

	//Run the partition so long as the low and high counters don't cross.
	while (low <= high) {

		//Move the low pointer until we find a value too large for this side.
		while (low <= high && vals[low]->finishtime <= vals[lowpos]->finishtime) low++;

		//Move the high pointer until we find a value too small for this side.
		while (high >= low && vals[high]->finishtime > vals[lowpos]->finishtime) high--;

		//Now that we've identified two values on the wrong side, swap them.
		if (low < high)
		   swap2(vals[low], vals[high]);
	}

	//Swap the partition element into it's correct location.
	swap2(vals[lowpos], vals[high]);

    //Return the index of the partition element.
	return high;
}

//Quicksort function
void quicksort(report** array, int low, int high) {

    //If array chunk is small enough, just do an insertion sort and collapse the recursion
    if((high - low) <= 10) {
        insertionSort(array, low, high);
        return;
    }

    //Check if array is already sorted
    if(is_sorted(array, low, high)) return;

    //Recursively sort the array until it's broken into a small enough segment to do an insertion sort
    int split = partition(array,low,high);
    quicksort(array,low,split-1);
    quicksort(array,split+1,high);
}

//Function to check if an array is in ascending order or all the same value
int is_sorted(report** array, int low, int high) {

    int i;

    // Return false if any adjacent pair is out of order.
    for (i=low; i<high-1; i++) {
        if (array[i]->finishtime > array[i+1]->finishtime) {
            return 0;
        }
    }

    return 1;
}

//Function to do an insertion sort for small arrays
void insertionSort(report** array, int low, int high) {

     int i,j;

     //Loop through each element to insert. If numbers are tied, select lower hair stylist station
     for(i=(low+1); i<=high; i++) {
         j=i;
         //Continue swapping the element until it hits the correct location.
         while (j > 0 && array[j]->finishtime < array[j-1]->finishtime) {
               swap2(array[j], array[j-1]);
               j--;
         }
         j = i;
         while( j > 0 && array[j]->finishtime == array[j-1]->finishtime) {
            if(array[j]->stylnum < array[j-1]->stylnum) {
                swap2(array[j], array[j-1]);
            }
            j--;
         }
     }
}

//Print contents of sorted output array
void printResults(report** array, int n) {
    for(int i = 0; i<n; i++) {
        printf("%s %d %d %s\n", array[i]->cusname, array[i]->finishtime, array[i]->finloyalty, array[i]->stylname);
    }
}
