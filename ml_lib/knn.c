#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "knn.h"

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

////////////////////////////////////// Main Algorithm Functions //////////////////////////////////

// External function called to run algorithm
int classify_knn(RPoint r, RPoint * training_data, int numTrainingPoints, int numFeatures, int numClasses){
    int k = 7;
    return classify_knn_internal(r,training_data, numTrainingPoints, numFeatures, numClasses, k);
}

// Steps for classification
// 1. Using input point, create Points from each RPoint of the training set
// 2. Sort Points
// 3. Take first k Points from the sorted list
// 4. Analyze first k Points
int classify_knn_internal(RPoint r, RPoint * training, int numTrainingPoints, int numFeatures, int numClasses, int k){

    Point points[numTrainingPoints];
    int classes[numClasses + 1], gesture = -1;
    size_t size_struct_points = sizeof(Point);

    for(int i = 0; i < numClasses + 1; i++)
        classes[i] = 0;

    for(int i = 0; i < numTrainingPoints; i++)
        points[i] = euclidean_distance(training[i], r, numFeatures);

    qsort(points, (size_t)numTrainingPoints, size_struct_points, compare);
    calculate_frequencies(points, classes, k);

    for(int i = 0; i < 5; i++)
//        printf("%f %f %f\n", r.data_x[i], r.data_y[i], r.data_z[i]);
//        printf("%d, %f\n", points[i].class, points[i].distance);

    gesture = determine_class(classes, numClasses);

//    printf("Gesture: %d\n", gesture);

    return gesture;
}

void calculate_frequencies(Point * sorted_points, int * classes, int k){
    for(int i = 0; i < k; i++)
        classes[sorted_points[i].class]++;
}

int determine_class(int * classes, int num){

    int count = 0;
    int answer = 0;

    for(int i = 0; i < num + 1; i++){
        if(classes[i] > count){
            count = classes[i];
            answer = i;
        }
    }

    return answer;
}


//////////////////////////////////// Helper Functions /////////////////////////////

// Compare function for Point structs, used for qsort
int compare(const void *v1, const void *v2){

    const Point *p1 = v1;
    const Point *p2 = v2;

    if(p1->distance > p2->distance) return 1;
    else if(p1->distance < p2->distance) return -1;
    else return 0;
}

// Dynamic Time Warping implementation. Better way to compare time-series data.
// Takes N^2 time wrt #features.
Point dtw(RPoint r1, RPoint r2, int size){

    double dtwCalc1[size+1][size+1];
    double dtwCalc2[size+1][size+1];
    double dtwCalc3[size+1][size+1];
    double infinity = 10000000;
    double cost1, cost2, cost3;
    double distance;
    Point p;

    for(int i = 0; i < size+1; i++){

        dtwCalc1[i][0] = infinity;
        dtwCalc1[0][i] = infinity;
        dtwCalc2[i][0] = infinity;
        dtwCalc2[0][i] = infinity;
        dtwCalc3[i][0] = infinity;
        dtwCalc3[0][i] = infinity;
    }

    dtwCalc1[0][0] = 0.0;
    dtwCalc2[0][0] = 0.0;
    dtwCalc3[0][0] = 0.0;

    for(int i = 0; i < size; i++){
        for(int j = 0; j < size; j++){
            cost1 = fabs(r1.data_x[i] - r2.data_x[j]);
            dtwCalc1[i+1][j+1] = cost1 + minimum(dtwCalc1[i][j+1], dtwCalc1[i+1][j], dtwCalc1[i][j]);
            cost2 = fabs(r1.data_y[i] - r2.data_y[j]);
            dtwCalc2[i+1][j+1] = cost2 + minimum(dtwCalc2[i][j+1], dtwCalc2[i+1][j], dtwCalc2[i][j]);
            cost3 = fabs(r1.data_z[i] - r2.data_z[j]);
            dtwCalc3[i+1][j+1] = cost3 + minimum(dtwCalc3[i][j+1], dtwCalc3[i+1][j], dtwCalc3[i][j]);
        }
    }

    distance = dtwCalc1[size][size] + dtwCalc2[size][size] + dtwCalc3[size][size];
    p.distance = distance;
    p.class = r1.class;

    return p;
}

// Calculate minimum of three doubles
double minimum(double a, double b, double c){
    if(a < b && a < c) return a;
    else if(b < a && b < c) return b;
    else return c;
}

Point euclidean_distance(RPoint r1, RPoint r2, int size){

    Point p;

    double squared_distance1 = 0.0;
    double squared_distance2 = 0.0;
    double squared_distance3 = 0.0;

    double distance1;
    double distance2;
    double distance3;

    for(int i = 1; i < size; i++){
        squared_distance1 = squared_distance1 + pow((r1.data_x[i] - r2.data_x[i]), 2);
        squared_distance2 = squared_distance2 + pow((r1.data_y[i] - r2.data_y[i]), 2);
        squared_distance3 = squared_distance3 + pow((r1.data_z[i] - r2.data_z[i]), 2);
    }

    distance1 = sqrt(squared_distance1);
    distance2 = sqrt(squared_distance2);
    distance3 = sqrt(squared_distance3);

    p.distance = distance1 + distance2 + distance3;
    p.class = r1.class;

    return p;
}

int normalize(RPoint * r, int numFeatures){

    double max_x = 0.0, max_y = 0.0, max_z = 0.0 ;

    for(int i = 0; i < numFeatures; i++){
      
        if(fabs(r[0].data_x[i]) > max_x)
            max_x = r[0].data_x[i];
        if(fabs(r[0].data_y[i]) > max_y)
            max_y = r[0].data_y[i];
        if(fabs(r[0].data_z[i]) > max_z)
            max_z = r[0].data_z[i];
    }

    for(int i = 0; i < numFeatures; i++){
        r[0].data_x[i] = r[0].data_x[i]/max_x;
        r[0].data_y[i] = r[0].data_y[i]/max_y;
        r[0].data_y[i] = r[0].data_y[i]/max_z;
    }
    return 0;
}

