package entity;

/**
 * Created by sulvto on 17-11-2.
 */
public interface EntityVisitor<T> {
    T visit(DefinedVariable variable);

    T visit(UndefinedVariable variable);

    T visit(DefinedFunction function);

    T visit(UndefinedFunction function);

    T visit(Constant constant);
}
