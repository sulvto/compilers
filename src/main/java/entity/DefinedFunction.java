package entity;

import ast.BlockNode;
import ast.Dumper;
import ast.TypeNode;

/**
 * Created by sulvto on 16-12-13.
 */
public class DefinedFunction extends Function {
    private Params params;
    private BlockNode body;

    public DefinedFunction(boolean isPrivate, TypeNode type, String name, Params params, BlockNode body) {
        super(isPrivate, type, name);
        this.params = params;
        this.body = body;
    }

    @Override
    public void dump(Dumper dumper) {
        dumper.printMember("name", name);
        dumper.printMember("params", params);
        dumper.printMember("body", body);
    }
}
