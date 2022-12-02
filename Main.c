/************************************************************************************************
 * CSC1003-Programming Methodology
 * Assignment by Session 3 Group 7
 * Main.c
 * This C program read the text file "fertility_Diagnosis_Data_Group5_8.txt".
 * The program would perform the following:
 *       - Contains a function that reads the file and store data into a multidimensional array
 *       - Contains a function that calculates Prior Probability
 *       - Contains a function that calculates Conditional Probability
 *       - Contains a function that calculates Standard Gaussian
 *       - Contains a function that calculates Posterior Probability,
 *         prints the probability of error and calculates the time taken for the program to run.
 *       - Contains a function that prints confusion matrix
 *
 ***********************************************************************************************/

#include <stdio.h>
#include <math.h> //for pow(),exp(),m_PI
#include <time.h> // for clock_t, clock(), CLOCKS_PER_SEC, clock_gettime()
#define NORMAL 0
#define ALTERED 1

// Global variables
FILE *fPointer;

// Function prototypes
int openFile(char fileName[]);
void setArrayValue(FILE *fp, float dataArray[10][10][10]);
int runCalculations(float dataset[10][10][10], int trainingSize, int testSize, double errorProb[2][5], int size[2][5], int i);
int getTotalOutcome(int dataSize, float dataArray[dataSize][10], int option);
double priorProbability(int outcomeCounter, int dataSize);
double conditionalProbability(int dataSize, float trainingArray[dataSize][10], int element, float option, int outcome, int optionCount);
double conditionalProbabilityGaussian(int dataSize, float trainingArray[dataSize][10], int element, float option, int outcome);
int posteriorProbability(int trainingSize, int testSize, float trainingArray[trainingSize][10], float testArray[testSize][10], double errorProb[2][5], int size[2][5], int i);
void confusionMatrix(int correctAltered, int correctNormal, int wrongAltered, int wrongNormal);
double probabilityError(int dataSize, int totalError);
void graph(double errorProb[2][5], int size[2][5]);

int main()
{
    float dataset[10][10][10];
    double errorProb[2][5];
    int sizeArray[2][5];

    // Opens the file to read from
    if (openFile("fertility_Diagnosis_Data_Group5_8.txt") == 1)
    {
        return 1;
    }

    // Get the values from the file and add it into the array dataset
    setArrayValue(fPointer, dataset);

    // Perform calculations with different ratio of the training and testing array
    for (int i = 50; i < 91; i += 10)
    {
        printf("\n---------------------------------------------------\n");
        printf("%*sNAIVE BAYES PROGRAM\n", 15, "");
        printf("---------------------------------------------------\n");
        static int y = 0;
        static int element = 0;
        printf("\n%*sData Set(Training:Testing)", 12, "");
        printf("\n%*s%d : %d\n", 21, "", i, i - y);
        runCalculations(dataset, i, i - y, errorProb, sizeArray, element);
        y += 20;
        element += 1;
    }

    // Call GNU plot function
    graph(errorProb, sizeArray);

    // Close the file
    fclose(fPointer);
    return 0;
}

// Function to open a file
int openFile(char filename[])
{
    if ((fPointer = fopen(filename, "r")) == NULL)
    {
        printf("Error! opening file");

        // Program exits if the file pointer returns NULL.
        return 1;
    }

    return 0;
}

// Function to perform all required calculations to obtain the prediction
int runCalculations(float dataset[10][10][10], int trainingSize, int testSize, double errorProb[2][5], int size[2][5], int i)
{
    clock_t begin = clock(); // start cpu time

    struct timespec start, end;            // define timepec structure from time.h
    clock_gettime(CLOCK_REALTIME, &start); // start wall clock time

    // Declare training arrary based on size in the parameter
    float trainingArray[trainingSize][10];
    int trainCounter = 0;

    // Store training data into 2d array according to the size of the training set
    for (int i = 0; i < (trainingSize / 10); i++)
    {
        for (int j = 0; j < 10; j++)
        {
            for (int k = 0; k < 10; k++)
            {
                trainingArray[trainCounter + j][k] = dataset[i][j][k];
            }
        }
        trainCounter += 10;
    }

    // Declare testing arrary based on size in the parameter
    float testArray[testSize][10];
    int testCounter = 0;

    // Store testing data into 2d array according to the size of the test set
    for (int i = (10 - (testSize / 10)); i < 10; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            for (int k = 0; k < 10; k++)
            {
                testArray[testCounter + j][k] = dataset[i][j][k];
            }
        }
        testCounter += 10;
    }

    // call posterior function
    posteriorProbability(trainingSize, testSize, trainingArray, testArray, errorProb, size, i);

    clock_t endTime = clock();           // end CPU time
    clock_gettime(CLOCK_REALTIME, &end); // end wall clock time

    double timeTaken = (double)(endTime - begin) / CLOCKS_PER_SEC;                                  // CPU time in seconds
    double time_spent = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0; // total number of seconds + totalnumber of nano second int terms of seconds

    printf("\n*********************TIME TAKEN*******************\n");
    printf("\n\tWall Clock Time Taken: %f seconds", time_spent);
    printf("\n\tCPU Time Taken: %f seconds\n", timeTaken);
    printf("___________________________________________________\n");
    printf("\n%*sEND OF %d:%d\n", 18, "", trainingSize, testSize);
    printf("___________________________________________________\n\n");
}

// Function to get the values from the file and add it into the array dataset
void setArrayValue(FILE *fp, float dataArray[10][10][10])
{
    // Continue the while loop till the file reaches the end
    while (!feof(fPointer))
    {
        // Store the float values into the array dataset
        for (int i = 0; i < 10; i++)
        {
            for (int j = 0; j < 10; j++)
            {
                for (int k = 0; k < 10; k++)
                {
                    fscanf(fp, "%f,", &dataArray[i][j][k]);
                }
            }
        }
    }
}

// Function to get the total number of times that specific feature appeared when normal or altered as the outcome
int getTotalOutcome(int dataSize, float dataArray[dataSize][10], int option)
{
    // printf("sda %d sd: %d\n", dataSize, dataArray[0][0]);
    int counter = 0; // The total number of times normal appeared in the training dataset.
    for (int i = 0; i < dataSize; i++)
    {
        if (dataArray[i][9] == option)
        {
            counter++;
        }
    }
    return counter;
}

// Getting the prior probability for normal outcome.
double priorProbability(int outcomeCounter, int dataSize)
{
    double pp = 0; // Prior probability for normal outcome.
    pp = (double)outcomeCounter / (double)dataSize;

    return pp;
}

// Code to find conditional probability for various inputs.
double conditionalProbability(int dataSize, float dataArray[dataSize][10], int element, float option, int outcome, int optionCount)
{
    double counter = 1;                                                                  // The number of times the selected input occurred in the datasets
    double conditionalProbability;                                                       // The conditional probability that is calculated.
    double outcomeCounter = getTotalOutcome(dataSize, dataArray, outcome) + optionCount; // The total amount of times the outcome appeared in the datasets.

    // Access the selected input in the array
    for (int i = 0; i < dataSize; i++)
    {
        if (dataArray[i][element] == option && dataArray[i][9] == outcome)
        {
            counter++;
        }
    }

    // Conditional probability formula
    conditionalProbability = counter / outcomeCounter;

    return conditionalProbability;
}

// Code to find posteriorProbability for normal and altered
int posteriorProbability(int trainingSize, int testSize, float trainingArray[trainingSize][10], float testArray[testSize][10], double errorProb[2][5], int size[2][5], int i)
{
    double testingNormal[testSize];
    double testingAltered[testSize];
    double trainingNormal[trainingSize];
    double trainingAltered[trainingSize];
    int totalTrainingError = 0;
    int totalTestingError = 0;

    int trainingCorrectAltered = 0;
    int trainingCorrectNormal = 0;
    int trainingWrongAltered = 0;
    int trainingWrongNormal = 0;

    int testCorrectAltered = 0;
    int testCorrectNormal = 0;
    int testWrongAltered = 0;
    int testWrongNormal = 0;

    int optionCount[9] = {4, -1, 2, 2, 2, 3, 5, 3, -1}; // for laplace smoothing

    // Print training and testing size
    printf("\n**********************OUTPUT***********************");
    printf("\n\nTraining Size: %d", trainingSize);
    printf("\nTest Size: %d", testSize);

    // run through training data
    for (int i = 0; i < trainingSize; i++)
    {
        // initialize posterior variable
        double normalProb = log(priorProbability(getTotalOutcome(trainingSize, trainingArray, NORMAL), trainingSize));
        double alteredProb = log(priorProbability(getTotalOutcome(trainingSize, trainingArray, ALTERED), trainingSize));

        // run though each feature of that row.
        for (int j = 0; j < 9; j++)
        {
            // if at feature 2 or 9 calculate gaussian
            if (j == 1 || j == 8)
            {
                normalProb += log(conditionalProbabilityGaussian(trainingSize, trainingArray, j, trainingArray[i][j], NORMAL));
                alteredProb += log(conditionalProbabilityGaussian(trainingSize, trainingArray, j, trainingArray[i][j], ALTERED));
            }
            // use conditional probability
            else
            {
                normalProb += log(conditionalProbability(trainingSize, trainingArray, j, trainingArray[i][j], NORMAL, optionCount[j]));
                alteredProb += log(conditionalProbability(trainingSize, trainingArray, j, trainingArray[i][j], ALTERED, optionCount[j]));
            }
        }
        // stores in posterior variable in array
        trainingNormal[i] = normalProb;
        trainingAltered[i] = alteredProb;

        // Count total number of error and false positives
        if ((trainingAltered[i] > trainingNormal[i]) && (trainingArray[i][9] != ALTERED)) // if predicted altered but actually normal
        {
            totalTrainingError++;
            trainingWrongAltered++;
        }
        // count the number of true positive
        else if ((trainingAltered[i] > trainingNormal[i]) && (trainingArray[i][9] == ALTERED)) // if predicted altered is correct
        {
            trainingCorrectAltered++;
        }
        // count the number of errors and false negative
        else if ((trainingAltered[i] < trainingNormal[i]) && (trainingArray[i][9] != NORMAL)) // if predicted normal but actually altered
        {
            totalTrainingError++;
            trainingWrongNormal++;
        }
        // count the number of true negative
        else if ((trainingAltered[i] < trainingNormal[i]) && (trainingArray[i][9] == NORMAL)) // if predict normal is correct
        {
            trainingCorrectNormal++;
        }
    }

    // print probability error for training
    printf("\n\n|||||||||||||||||||||TRAINING||||||||||||||||||||||\n");
    printf("\nnumber of error for training is %d\n", totalTrainingError);
    printf("Probability of error for training is %f\n", probabilityError(trainingSize, totalTrainingError));
    printf("Probability of error for training in pecentage: %0.3f%%\n", probabilityError(trainingSize, totalTrainingError) * 100);
    printf("\n*************CONFUSION MATRIX:TRAINING*************\n");
    confusionMatrix(trainingCorrectAltered, trainingCorrectNormal, trainingWrongAltered, trainingWrongNormal);

    errorProb[0][i] = probabilityError(trainingSize, totalTrainingError) * 100;
    size[0][i] = trainingSize;

    // run through testing data
    for (int i = 0; i < testSize; i++)
    {
        double normalProb = log(priorProbability(getTotalOutcome(trainingSize, trainingArray, NORMAL), trainingSize));
        double alteredProb = log(priorProbability(getTotalOutcome(trainingSize, trainingArray, ALTERED), trainingSize));

        for (int j = 0; j < 9; j++)
        {
            if (j == 1 || j == 8)
            {
                normalProb += log(conditionalProbabilityGaussian(trainingSize, trainingArray, j, testArray[i][j], NORMAL));
                alteredProb += log(conditionalProbabilityGaussian(trainingSize, trainingArray, j, testArray[i][j], ALTERED));
            }
            else
            {
                normalProb += log(conditionalProbability(trainingSize, trainingArray, j, testArray[i][j], NORMAL, optionCount[j]));
                alteredProb += log(conditionalProbability(trainingSize, trainingArray, j, testArray[i][j], ALTERED, optionCount[j]));
            }
        }

        // stores in posterior variable in array
        testingNormal[i] = normalProb;
        testingAltered[i] = alteredProb;

        // Count total number of error and false positives
        if ((testingAltered[i] > testingNormal[i]) && (testArray[i][9] != 1))
        {
            totalTestingError++;
            testWrongAltered++;
        }
        // count the number of true positive
        else if ((testingAltered[i] > testingNormal[i]) && (testArray[i][9] == 1))
        {
            testCorrectAltered++;
        }
        // count the number of errors and false negative
        else if ((testingAltered[i] < testingNormal[i]) && (testArray[i][9] != 0))
        {
            totalTestingError++;
            testWrongNormal++;
        }
        // count the number of true negative
        else if ((testingAltered[i] < testingNormal[i]) && (testArray[i][9] == 0))
        {
            testCorrectNormal++;
        }
    }

    // print probability error for testing
    printf("\n\n||||||||||||||||||||||TESTING||||||||||||||||||||||\n");
    printf("\nnumber of error for testing is %d\n", totalTestingError);
    printf("Probability of error for testing: %f\n", probabilityError(testSize, totalTestingError));
    printf("Probability of error for testing in pecentage: %0.3f%%\n", probabilityError(testSize, totalTestingError) * 100);
    printf("\n*************CONFUSION MATRIX:TESTING*************\n");
    confusionMatrix(testCorrectAltered, testCorrectNormal, testWrongAltered, testWrongNormal);

    errorProb[1][i] = probabilityError(testSize, totalTestingError) * 100;
    size[1][i] = testSize;
    return 0;
}

// function that returns conditional probability for gaussian
double conditionalProbabilityGaussian(int dataSize, float dataArray[dataSize][10], int element, float option, int outcome)
{
    double conditionalProbabilityGaussian = 0;
    double mean = 0;
    double variance = 0;
    double sumMean = 0;
    double sumVariance = 0;
    int counter = 0;

    // calculate the total value of feature 2/9 if outcome is either normal or altered
    for (int i = 0; i < dataSize; i++)
    {
        if (dataArray[i][9] == outcome)
        {
            sumMean += dataArray[i][element];
            counter++;
        }
    }
    // calculate mean of feature 2/9
    mean = sumMean / counter;

    //(X2,i - mean)^2 , (X9,i - mean)^2
    for (int i = 0; i < dataSize; i++)
    {
        if (dataArray[i][9] == outcome)
        {
            sumVariance += pow((dataArray[i][element] - mean), 2);
        }
    }

    // calculate varience
    variance = (sumVariance) / (counter - 1);

    // calculate standard Gaussian
    double z = (option - mean) / sqrt(variance);
    conditionalProbabilityGaussian = ((1 / (sqrt(2 * M_PI))) * exp(-0.5 * pow(z, 2)));

    return conditionalProbabilityGaussian;
}
// function that prints confusion matrix
void confusionMatrix(int correctAltered, int correctNormal, int wrongAltered, int wrongNormal)
{
    int spacingCorrectAltered = 6;
    int spacingWrongNormal = 6;
    if (correctAltered > 9)
    {
        spacingCorrectAltered = 5;
    }
    if (wrongNormal > 9)
    {
        spacingWrongNormal = 5;
    }

    printf("___________________________________________________\n");
    printf("\n%*sReal Positive   Real Negative\n", 24, "");
    printf("\n%*sPredicted Positive %*s%d%*s|%*s%d%*s", 5, "", 7, "", correctAltered, spacingCorrectAltered, "", 7, "", wrongAltered, 6, "");
    printf("\n%*s --------------|--------------", 23, "");

    printf("\n%*sPredicted Negative %*s%d%*s|%*s%d%*s", 5, "", 7, "", wrongNormal, spacingWrongNormal, "", 7, "", correctNormal, 6, "");
    printf("\n___________________________________________________\n");
}
// calculate probability Error
double probabilityError(int dataSize, int totalError)
{
    return (double)totalError / (double)dataSize;
}

// GNU plot function
void graph(double errorProb[2][5], int size[2][5])
{
    FILE *gnuplot;
    gnuplot = popen("gnuplot -persistent", "w");
    if (gnuplot != NULL)
    {
        fprintf(gnuplot, "set title 'Error probability' \n");                                          // Title for the graph
        fprintf(gnuplot, "set xlabel 'Data Size (Training:Testing)' \n");                              // X axis is the data size (training:testing)
        fprintf(gnuplot, "set ylabel 'Error probability (%%)' \n");                                    // Y axis is the error probability for the respective training size
        fprintf(gnuplot, "set xrange [45:95] \n");                                                     // X axis range
        fprintf(gnuplot, "set yrange [0:30] \n");                                                      // Y axis range
        fprintf(gnuplot, "set xtics ('50:50' 50, '60:40' 60, '70:30' 70, '80:20' 80, '90:10' 90) \n"); // Custom labels for x axis
        fprintf(gnuplot, "set grid \n");                                                               // Draws grid in the graph
        fprintf(gnuplot, "set terminal qt size 1920,1080 \n");                                         // Set a fixed size

        for (int i = 0; i < 2; i++)
        {
            for (int j = 0; j < 5; j++)
            {
                fprintf(gnuplot, "set object circle at first %d,%f radius char 0.5 \n", size[0][j], errorProb[i][j]);                   // Plotting circle on the points
                fprintf(gnuplot, "set label '(%d,%.2f%%)' at %d, %f \n", size[i][j], errorProb[i][j], size[0][j], errorProb[i][j] + 1); // Labelling every point
            }
        }

        fprintf(gnuplot, "$training << EOD \n"); // Creating a data block for training set

        for (int j = 0; j < 5; j++)
        {

            fprintf(gnuplot, "%d %f \n", size[0][j], errorProb[0][j]); // Inserting the error probability into the training set data block
        }

        fprintf(gnuplot, "EOD \n"); // End of delimeter

        fprintf(gnuplot, "$testing << EOD \n"); // Creating another data block for testing set

        for (int j = 0; j < 5; j++)
        {

            fprintf(gnuplot, "%d %f \n", size[0][j], errorProb[1][j]); // Inserting the error probability into the testing set data block
        }

        fprintf(gnuplot, "EOD \n"); // End of delimeter

        // Plotting the graph
        fprintf(gnuplot, "plot '$training' title 'Training Set' lt rgb 'red' smooth csplines, '$testing' title 'Testing Set' lt rgb 'blue' smooth csplines \n");
    }

    fclose(gnuplot);
}
