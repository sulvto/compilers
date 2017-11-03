package ir;

/**
 * Created by sulvto on 17-11-4.
 */
public interface IRVisitor<S, E> {
    S visit(ExprStmt s);

    S visit(Assign s);

    S visit(Cjump s);

    S visit(Jump s);

    //    public S visit(Sw s);
    S visit(LabelStmt s);

    S visit(Ret s);


    E visit(Uni s);

    E visit(Bin s);

    E visit(Call s);

    E visit(Addr s);

    E visit(Mem s);

    E visit(Var s);

    E visit(Int s);

    E visit(Str s);
}
