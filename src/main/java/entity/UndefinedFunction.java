package entity;

import ast.Dumper;
import ast.TypeNode;
import lexer.Token;

import java.util.List;

/**
 * Created by sulvto on 17-11-2.
 */
public class UndefinedFunction extends Function {
    private Params params;

    public UndefinedFunction(TypeNode type, Token name, Params params) {
        super(false, type, name);
        this.params = params;

    }

    public List<Parameter> parameters() {
        return this.params.parameterList;
    }

    @Override
    public boolean isDefined() {
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
