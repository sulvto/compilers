package entity;

import ast.Dumper;
import ast.TypeNode;
import lexer.Token;

/**
 * Created by sulvto on 17-11-2.
 */
public class UndefinedVariable extends Variable {
    public UndefinedVariable(TypeNode type, Token name) {
        super(false, type, name);
    }

    @Override
    public boolean isDefined() {
        return false;
    }

    public boolean isInitialized() {
        return false;
    }

    @Override
    public <T> T accept(EntityVisitor<T> visitor) {
        return visitor.visit(this);
    }

    @Override
    public void dump(Dumper dumper) {

    }
}
