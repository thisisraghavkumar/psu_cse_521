def eliminate_last(A, b):
    """
    Eliminate the last variable from a system of inequalities

    Parameters
    --------------------
    Matrix `A` of size m x n from expression Ax leq b

    Vector `b` of size m x 1 from expression Ax leq b

    Returns
    --------------------
    A three tuple containing

    Tuple `(A',b')` for the new system of equation

    List of list `[c_1,c_2...c_{n-1},b]` representing the upper bounds of the
    last variable

    List of list `[c_1,c_2...c_{n-1},b]` representing the lower bound of the
    last variable
    """
    m = len(A)
    # there are no constraints to solve
    if m == 0:
        return ([], []), [], []
    n = len(A[0])
    UB = []
    LB = []
    A_prime = []
    b_prime = []
    for r in range(m):
        c_n = A[r][n-1]
        d = abs(c_n)
        if d == 0:
            A_prime.append(A[r])
            continue
        d = float(d)
        for c in range(n):
            A[r][c] /= d
        b[r] /= d
        c_n = A[r][n-1]
        if c_n == -1:
            t = A[r][:n-1]
            t.append(-1 * b[r])
            LB.append(t)
        else:
            t = [-1 * x for x in A[r][:n-1]]
            t.append(b[r])
            UB.append(t)
    for r1 in range(m):
        for r2 in range(r1+1, m):
            if A[r1][n-1] != A[r2][n-1]:
                t = [A[r1][c] + A[r2][c] for c in range(n-1)]
                A_prime.append(t)
                b_prime.append(b[r1]+b[r2])
    return (A_prime, b_prime), UB, LB


def printMatrix(M):
    if len(M) == 0:
        return
    n = len(M[0])
    for row in M:
        for i, val in enumerate(row):
            print(f"{val:6.2f}", end="")
            if i != n-1:
                print(" ", end="")
        print()


if __name__ == '__main__':
    A = [[-3, -4], [4, 7], [4, -7], [-2, 3]]
    b = [-16, 56, 20, 9]
    (a_prime, b_prime), ub, lb = eliminate_last(A, b)
    printMatrix(a_prime)
    print("------------------")
    printMatrix([b_prime])
    print("------------------")
    printMatrix(ub)
    print("------------------")
    printMatrix(lb)
    print("*************************")
    (a_prime, b_prime), ub, lb = eliminate_last(a_prime, b_prime)
    printMatrix(a_prime)
    print("------------------")
    printMatrix([b_prime])
    print("------------------")
    printMatrix(ub)
    print("------------------")
    printMatrix(lb)
