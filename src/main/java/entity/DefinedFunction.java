package entity;

import ast.BlockNode;
import ast.Dumper;
import ast.TypeNode;

import java.util.List;

/**
 * Created by sulvto on 16-12-13.
 */
public class DefinedFunction extends Function {
    private Params params;
    private BlockNode body;
    private LocalScope scope;

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

    public List<Parameter> parameter() {
        return params.parameterList;
    }

    public BlockNode getBody() {
        return body;
    }

    @Override
    public String location() {
        return "";
    }

    public void setScope(LocalScope scope) {
        this.scope = scope;
    }
}
