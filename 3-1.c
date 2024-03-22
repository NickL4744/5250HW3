#include <stdio.h>
#include <omp.h>

int main() {
    int n = 1000000; // Number of terms in the series
    double sum = 0.0;
    double factor = 1.0;

    #pragma omp parallel for reduction(+:sum)
    for (int k = 0; k < n; k++) {
        if (k % 2 == 0) {
            factor = 1.0;
        } else {
            factor = -1.0;
        }
        sum += factor / (2 * k + 1);
    }

    double pi = 4.0 * sum;
    printf("Estimated value of pi: %.10f\n", pi);

    return 0;
}