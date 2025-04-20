def eliminate_last(A, b):
    """
    Eliminate the last variable (x_n) from a system of inequalities

    Parameters
    --------------------
    Matrix `A` of size m x n from expression Ax leq b

    Vector `b` of size m x 1 from expression Ax leq b

    Returns
    --------------------
    A five tuple containing

    Tuple `(A',b')` for the new system of equation

    List of list `[c_1,c_2...c_{n-1},b]` representing the upper bounds of the
    last variable

    List of list `[c_1,c_2...c_{n-1},b]` representing the lower bound of the
    last variable

    Boolean `unsolvable` if some of the constraint is unsolvable

    Boolean `exact_projection` if the projection for bounds of variable `x_n`
    being eliminated were exact i.e. its coefficient was 1 for all inequalities
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
    exact_projection = True
    for r in range(m):
        c_n = A[r][n-1]
        d = abs(c_n)
        if d == 0:
            # Nothing for the bounds of x_n here
            continue
        if d != 1:
            exact_projection = False
        d = float(d)
        c_n = A[r][n-1]
        if c_n < 0:
            t = [float(v)/d for v in A[r][:n-1]]
            t.append(-1 * (b[r]/d))
            LB.append(t)
        else:
            t = [-1 * (float(x)/d) for x in A[r][:n-1]]
            t.append(b[r]/d)
            UB.append(t)
    for r1 in range(m):
        if A[r1][n-1] == 0:
            A_prime.append(A[r1][:n-1])
            b_prime.append(b[r1])
            continue
        for r2 in range(r1+1, m):
            if A[r1][n-1] * A[r2][n-1] < 0:
                t = [abs(A[r2][n-1]) * A[r1][c] + abs(A[r1][n-1]) * A[r2][c] for c in range(n-1)]
                all_coeff_zero = True
                for c in t:
                    all_coeff_zero = all_coeff_zero and (c == 0)
                unsolvable = unsolvable or (all_coeff_zero and (
                    abs(A[r2][n-1])*b[r1] + abs(A[r1][n-1])*b[r2] < 0))
                A_prime.append(t)
                b_prime.append(abs(A[r2][n-1]) * b[r1]+abs(A[r1][n-1]) * b[r2])
    return (A_prime, b_prime), UB, LB, unsolvable, exact_projection


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


def get_affine_exp(C):
    exp = ""
    n = len(C)
    for i in range(n-1):
        if exp == "":
            exp += f"{C[i]:.2f}*x_{i+1}"
        else:
            exp += f" {"+" if C[i] >= 0 else "-"} {abs(C[i]):.2f}*x_{i+1}"
    if exp == "":
        exp += f"{C[n-1]:.2f}"
    else:
        exp += f" {"+" if C[n-1] > 0 else "-"} {abs(C[n-1]):.2f}"
    return exp


def get_bounds_expression(C, type):
    res = ""
    if C is None or len(C) == 0:
        if type == "lower": res = "-5000000000"
        if type == "upper": res = "5000000000"
    else:
        t = ",".join([get_affine_exp(row) for row in C])
        if type == "lower":
            res = "max(["+t+"])"
        else:
            res = "min(["+t+"])"
    return res


def get_loop_nest(LBs, UBs, n):
    res = ""
    tabs = 0
    for i in range(0, n):
        res += " "*tabs
        tabs += 4
        lower_bounds = get_bounds_expression(LBs[n-i-1], "lower")
        upper_bounds = get_bounds_expression(UBs[n-i-1], "upper")
        res += f"for x_{i+1} in range(math.ceil({lower_bounds}),math.floor({upper_bounds})+1):\n"
    res += " "*tabs
    res += f"print({",".join([f"x_{i+1}" for i in range(n)])})"
    return res


if __name__ == '__main__':
    n = int(input("Number of variables: "))
    ndim = n
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
    projections = []
    print("---------Steps------------")
    while n > 0:
        print(f"Eliminating x_{n}")
        (a_prime, b_prime), ub, lb, us, ep = eliminate_last(arg_A, arg_b)
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
        print("*****************************************************")
        if ep:
            projections.append("exact")
        else:
            projections.append("inexact")
        arg_A = a_prime
        arg_b = b_prime
        m = len(arg_A)
        n -= 1
        if us:
            break
    if us:
        print("\nSolution does not exist!")
    else:
        print("\nProjections -")
        for i, p in enumerate(projections):
            print(f"x_{i+1}: {p}")
        x_1_min = max(LBs[-1])[0] if (len(LBs[-1]) > 0) and (LBs[-1][0] is not list) else -500000000
        x_1_max = min(UBs[-1])[0] if (len(UBs[-1]) > 0) and (UBs[-1][0] is not list) else 500000000
        print(f"\n{x_1_min:.2f} <= x_1 <= {x_1_max:.2f}")
        if x_1_max >= x_1_min and (x_1_max - x_1_min >= 1):
            print("Solutions exists\n")
            res = get_loop_nest(LBs, UBs, ndim)
            print("\nPython loop nest:\n\nimport math\n"+res)
        else:
            print("\nSolution does not exist")
