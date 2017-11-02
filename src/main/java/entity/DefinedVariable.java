package entity;

import ast.Dumper;
import ast.ExprNode;
import ast.TypeNode;
import lexer.Token;

/**
 * Created by sulvto on 16-12-13.
 */
public class DefinedVariable extends Variable {
    private ExprNode initializer;

    public DefinedVariable(boolean isPrivate, TypeNode type, Token token, ExprNode init) {
        super(isPrivate, type, token);
        this.initializer = init;
    }

    public ExprNode initializer() {
        return initializer;
    }

    @Override
    public void dump(Dumper dumper) {
        dumper.printMember("name", name.value);
        dumper.printMember("typeNode", typeNode);
        dumper.printMember("initializer", initializer);
    }

    @Override
    public String location() {
        return null;
    }

    public boolean hasInitializer() {
        return initializer != null;
    }

}
