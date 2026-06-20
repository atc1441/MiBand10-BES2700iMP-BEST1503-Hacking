
// Calculate:
//   C[M][N] = AT[K][M] * B[K][N]
//
// Output
//   32 bit
//   16 bit
//    8 bit
template<typename Ta, typename Tb, typename Tc>
void naive_mmult_aTb(const Ta *A, const Tb *B, Tc *C, int M, int N, int K)
{
    int i,j,p;
    Tc dot_p;

    for (i = 0; i < M; i++) {
      for (j = 0; j < N; j++) {
        // Calculate the dot product between A.column and B.column:
        dot_p = 0;
        for (p = 0; p < K; p++) {
           dot_p += A[i+p*M] * B[p*N+j]; // A[M][K] * B[K][N]
        }
        C[i*N+j] = dot_p;
      }
    }
}


// Calculate:
//   C[M][N] = A[M][K] * B[K][N]
//
// Output
//   32 bit
//   16 bit
//    8 bit
template<typename Ta, typename Tb, typename Tc>
void naive_mmult_ab(const Ta *A, const Tb *B, Tc *C, int M, int N, int K)
{
    int i,j,p;
    Tc dot_p;

    for (i = 0; i < M; i++) {
      for (j = 0; j < N; j++) {
        // Calculate the dot product between A.row and B.column:
        dot_p = 0;
        for (p = 0; p < K; p++) {
           dot_p += A[i*K+p] * B[p*N+j]; // A[M][K] * B[K][N]
        }
        C[i*N+j] = dot_p;
      }
    }
}


// Calculate:
//   C[M][N] = AT[K][M] * BT[N][K]
//
// Output
//   32 bit
//   16 bit
//    8 bit
template<typename Ta, typename Tb, typename Tc>
void naive_mmult_aTbT(const Ta *A, const Tb *B, Tc *C, int M, int N, int K)
{
    int i,j,p;
    Tc dot_p;

    for (i = 0; i < M; i++) {
      for (j = 0; j < N; j++) {
        // Calculate the dot product between A.row and B.column:
        dot_p = 0;
        for (p = 0; p < K; p++) {
           dot_p += A[i+p*M] * B[p+j*K]; // AT[K][M] * BT[N][K]
        }
        C[i*N+j] = dot_p;
      }
    }
}

// Calculate:
//   C[M][N] = A[M][K] * BT[N][K]
//
// Output
//   32 bit
//   16 bit
//    8 bit
template<typename Ta, typename Tb, typename Tc>
void naive_mmult_abT(const Ta *A, const Tb *B, Tc *C, int M, int N, int K)
{
    int i,j,p;
    Tc dot_p;

    for (i = 0; i < M; i++) {
      for (j = 0; j < N; j++) {
        // Calculate the dot product between A.row and B.column:
        dot_p = 0;
        for (p = 0; p < K; p++) {
           dot_p += A[i*K+p] * B[p+j*K]; // AT[K][M] * BT[N][K]
        }
        C[i*N+j] = dot_p;
      }
    }
}

