package ast;

/**
 * Created by sulvto on 17-11-2.
 */
public interface ASTVisitor<S, E> {
    S visit(BlockNode node);

    S visit(ExprStmtNode node);

    S visit(IfNode node);

    S visit(SwitchNode node);

    S visit(CaseNode node);

    S visit(WhileNode node);

    S visit(DoWhileNode node);

    S visit(ForNode node);

    S visit(BreakNode node);

    S visit(ContinueNode node);

    //    public S visit(GotoNode node);
//    public S visit(LabelNode node);
    S visit(ReturnNode node);


    E visit(AssignNode node);

    E visit(OpAssignNode node);

    E visit(CondExprNode node);

    E visit(LogicalAndNode node);

    E visit(LogicalOrNode node);

    E visit(BinaryOpNode node);

    E visit(UnaryOpNode node);

    //    public E visit(PrefixOpNode node);
//    public E visit(SuffixOpNode node);
//    public E visit(ArefNode node);
//    public E visit(MemberNode node);
//    public E visit(PtrMemberNode node);
    E visit(FuncallNode node);

    //    public E visit(DereferNode node);
//    public E visit(AddressNode node);
//    public E visit(CastNode node);
//    public E visit(SizeOf node);
//    public E visit(Sizeof node);
    E visit(VariableNode node);

    E visit(IntegerLiteralNode node);

    E visit(StringLiteralNode node);
}
