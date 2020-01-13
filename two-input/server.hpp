#include <iostream>
#include <cstdio>
#include <fstream>
#include <chrono>
#include <random>
#include <vector>
#include <thread>
#include <future>
#include <ctime>
#include <ratio>
#include <math.h>
#include <cstddef>
#include <iomanip>
#include <string>
#include <mutex>
#include <memory>
#include <limits>
#include <thread>
#include <stdlib.h>
#include <stdio.h>
#include "seal/seal.h"
#include "omp.h"
#define NUM 4096 //The number of possible inputs for each input value
#define NUM2 16777216 //The number of all possible combination for two input values
#define NF 8 //The number of threads
