package ir;

/**
 * Created by sulvto on 17-11-4.
 */
public enum Op {
    ADD, SUB, MUL, S_DIV, U_DIV, S_MOD, U_MOD, BIT_AND, BIT_OR, BIT_XOR, BIT_LSHIFT, BIT_RSHIFT, ARITH_RSHIFT,

    EQ, NEQ, S_GT, S_GTEQ, S_LT, S_LTEQ, U_GT, U_GTEQ, U_LT, U_LTEQ,

    UMINUS, BIT_NOT, NOT,

    S_CAST, U_CAST;

    static public Op internBinary(String op, boolean isSigned) {
        switch (op) {
            case "+":
                return Op.ADD;
            case "-":
                return Op.SUB;
            case "*":
                return Op.MUL;
            case "/":
                return isSigned ? Op.S_DIV : Op.U_DIV;
            case "%":
                return isSigned ? Op.S_MOD : Op.U_MOD;
            case "&":
                return Op.BIT_AND;
            case "|":
                return Op.BIT_OR;
            case "^":
                return Op.BIT_XOR;
            case "<<":
                return Op.BIT_LSHIFT;
            case ">>":
                return isSigned ? Op.ARITH_RSHIFT : Op.BIT_RSHIFT;
            case "==":
                return Op.EQ;
            case "!=":
                return Op.NEQ;
            case "<":
                return isSigned ? Op.S_LT : Op.U_LT;
            case "<=":
                return isSigned ? Op.S_LTEQ : Op.U_LTEQ;
            case ">":
                return isSigned ? Op.S_GT : Op.U_GT;
            case ">=":
                return isSigned ? Op.S_GTEQ : Op.U_GTEQ;
            default:
                throw new Error("unknown binary op:" + op);
        }
    }

    public static Op internUnary(String op) {
        switch (op) {
            case "+":
                throw new Error("unary+ should not be in IR");
            case "-":
                return Op.UMINUS;
            case "~":
                return Op.BIT_NOT;
            case "!":
                return Op.NOT;
            default:
                throw new Error("unknown unary op:" + op);
        }

    }
}
