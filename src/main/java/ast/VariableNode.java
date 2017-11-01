package ast;

import entity.Entity;
import lexer.Token;

/**
 * Created by sulvto on 16-12-17.
 */
public class VariableNode extends LHSNode {
    private String name;
    private Entity entity;

    public VariableNode(Token token, String name) {
        super(token);
        this.name = name;
    }

    public String name() {
        return name;
    }

    @Override
    protected void doDump(Dumper dumper) {
        dumper.printMember("name", name);
    }

    @Override
    public <S, E> E accept(ASTVisitor<S, E> visitor) {
        return visitor.visit(this);
    }

    public void setEntity(Entity entity) {
        this.entity = entity;
    }
}
