def eliminate_last(A, b):
    """
    Eliminate the last variable from a system of inequalities

    Parameters
    --------------------
    Matrix `A` of size m x n from expression Ax leq b

    Vector `b` of size m x 1 from expression Ax leq b

    Returns
    --------------------
    A four tuple containing

    Tuple `(A',b')` for the new system of equation

    List of list `[c_1,c_2...c_{n-1},b]` representing the upper bounds of the
    last variable

    List of list `[c_1,c_2...c_{n-1},b]` representing the lower bound of the
    last variable

    Boolean `unsolvable` if some of the constraint is unsolvable
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
    unsolvable = False
    for r in range(m):
        c_n = A[r][n-1]
        d = abs(c_n)
        if d == 0:
            # No limits for x_n here
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
        if A[r1][n-1] == 0:
            A_prime.append(A[r1][:n-1])
            b_prime.append(b[r1])
            continue
        for r2 in range(r1+1, m):
            if A[r1][n-1] * A[r2][n-1] == -1:
                t = [A[r1][c] + A[r2][c] for c in range(n-1)]
                all_coeff_zero = True
                for c in t:
                    all_coeff_zero = all_coeff_zero and (c == 0)
                unsolvable = unsolvable or (all_coeff_zero and (b[r1] + b[r2] < 0))
                A_prime.append(t)
                b_prime.append(b[r1]+b[r2])
    return (A_prime, b_prime), UB, LB, unsolvable


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
    n = int(input("Number of variables: "))
    m = int(input("Number of constraints: "))
    ip = input(f"Enter {n}x{m} = {n*m} space seperated entries of M in row major order:")
    ip = ip.split(" ")
    print(ip)
    ip = [float(i) for i in ip]
    A = [ip[i:i+n] for i in range(0, n*m, n)]
    printMatrix(A)
    ip = input(f"Enter {m} space separated entries of b :")
    ip = ip.split(" ")
    ip = [float(i) for i in ip]
    b = ip
    printMatrix([b])
    LBs = []
    UBs = []
    arg_A = A
    arg_b = b
    print("---------Solution------------")
    while n > 0:
        (a_prime, b_prime), ub, lb, us = eliminate_last(arg_A, arg_b)
        LBs.append(lb)
        UBs.append(ub)
        print("---------A'--------------------")
        printMatrix(a_prime)
        print("---------b'--------------------")
        printMatrix([b_prime])
        print(f"--------x_{n} >= lb \\dot [{",".join([f"x_{i}" for i in range(1, n)])}{"," if n >1 else ""}{1}]------------")
        printMatrix(lb)
        print(f"--------x_{n} <= ub \\dot [{",".join([f"x_{i}" for i in range(1, n)])}{"," if n >1 else ""}{1}]------------")
        printMatrix(ub)
        print("***********")
        arg_A = a_prime
        arg_b = b_prime
        m = len(arg_A)
        n -= 1
        if us:
            break
    if us:
        print("\nSolution does not exist!")
    else:
        x_1_min = max(LBs[-1])[0] if (len(LBs[-1]) > 0) and (LBs[-1][0] is not list) else -500000000
        x_1_max = min(UBs[-1])[0] if (len(UBs[-1]) > 0) and (UBs[-1][0] is not list) else 500000000
        print(f"\n{x_1_min:.2f} <= x_1 <= {x_1_max:.2f}")
        if x_1_max >= x_1_min:
            print("Solutions exists\n")
        else:
            print("\nSolution does not exist")
