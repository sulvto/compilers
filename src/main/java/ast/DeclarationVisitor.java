package ast;

/**
 * Created by sulvto on 17-11-2.
 */
public interface DeclarationVisitor<T> {
    T visit(StructNode structNode);

    T visit(UnionNode unionNode);

    T visit(TypedefNode typedefNode);
}
