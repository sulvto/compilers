package entity;

import ast.Dumper;
import ast.ExprNode;
import ast.TypeNode;

/**
 * Created by sulvto on 16-12-13.
 */
public class DefinedVariable extends Variable {
    private ExprNode initializer;

    public DefinedVariable(boolean isPrivate, TypeNode type, String name, ExprNode init) {
        super(isPrivate, type, name);
        this.initializer = init;
    }

    @Override
    public void dump(Dumper dumper) {
        dumper.printMember("name", name);
        dumper.printMember("typeNode", typeNode);
        dumper.printMember("initializer", initializer);
    }
}
