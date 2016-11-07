package inter;

/**
 * Created by sulvto on 16-11-7.
 */
public class Break extends Stmt {
    Stmt stmt;

    public Break() {
        if (Stmt.Enclosing == Stmt.Null) error("unenclosed error");
        stmt = Stmt.Null;
    }

    @Override
    public void gen(int b, int a) {
        emit("goto L" + stmt.after);
    }
}
