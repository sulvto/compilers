package entity;

import ast.Dumper;
import ast.ExprNode;
import ast.TypeNode;
import lexer.Token;

/**
 * Created by sulvto on 16-12-13.
 */
public class Constant extends Entity {
    private ExprNode value;

    public Constant(boolean isPrivate, TypeNode type, Token name, ExprNode value) {
        super(isPrivate, type, name);
        this.value = value;
    }

    @Override
    public void dump(Dumper dumper) {
        // TODO
    }

    public ExprNode value() {
        return value;
    }
}
