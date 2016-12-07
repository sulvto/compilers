package inter;

import lexer.Token;
import symbols.Type;

/**
 * Created by sulvto on 16-11-6.
 */
public class Expr extends Node {
    public Token op;
    public Type type;

    Expr(Token token, Type p) {
        op = token;
        type = p;
    }

    public Expr gen() {
        return this;
    }

    public Expr reduce() {
        return this;
    }

    public void jumping(int t, int f) {
        emitjumps(toString(), t, f);
    }

    public void emitjumps(String test, int t, int f) {
        if (t != 0 && f != 0) {
            emit("if " + test + " goto L" + t);
            emit("goto L" + f);
        }
        else if (t != 0) emit("if " + test + " goto L" + t);
        else if (f != 0) emit("iffalse " + test + " goto L" + f);
        else ;
    }

    @Override
    public String toString() {
        return op.toString();
    }
}
