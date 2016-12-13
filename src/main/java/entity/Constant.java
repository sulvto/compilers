package entity;

import ast.Dumper;
import ast.ExprNode;
import ast.TypeNode;

/**
 * Created by sulvto on 16-12-13.
 */
public class Constant extends Entity {
    private ExprNode value;

    public Constant(boolean isPrivate, TypeNode type, String name, ExprNode value) {
        super(isPrivate, type, name);
        this.value = value;
    }

    @Override
    public void dump(Dumper dumper) {
        // TODO
    }
}
