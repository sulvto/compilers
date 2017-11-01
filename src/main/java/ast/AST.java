package ast;

import entity.*;

import java.util.ArrayList;
import java.util.List;

/**
 * ast root
 * Created by sulvto on 16-12-8.
 */
public class AST extends Node {

    private Declarations declarations;
    private ToplevelScope scope;
    private ConstantTable constantTable;

    public AST(Declarations declarations) {
        super(null);
        this.declarations = declarations;
    }

    public List<Entity> declarations() {
        List<Entity> result = new ArrayList<>();
        result.addAll(declarations.defvars());
        result.addAll(declarations.defuns());
        return result;
    }

    public List<Entity> definitions() {
        List<Entity> result = new ArrayList<>();
        result.addAll(declarations.defvars());
        result.addAll(declarations.defuns());
        result.addAll(declarations.constants());
        return result;
    }

    public List<Constant> constants() {
        return declarations.constants();
    }

    public List<DefinedVariable> definedVariables() {
        return declarations.defvars();
    }

    public List<DefinedFunction> definedFunctions() {
        return declarations.defuns();
    }


    @Override
    protected void doDump(Dumper dumper) {
        dumper.printMember("variables", definedVariables());
        dumper.printMember("functions", definedFunctions());
    }

    public void setScope(ToplevelScope scope) {
        if (this.scope != null) {
            throw new Error("must not happen: ToplevelScope set twice");
        }
        this.scope = scope;
    }

    public ToplevelScope getScope() {
        if (this.scope == null) {
            throw new Error("must not happen: AST.scope is null");
        }
        return scope;
    }

    public void setConstantTable(ConstantTable constantTable) {
        if (this.constantTable != null) {
            throw new Error("must not happen: ConstantTable set twice");
        }
        this.constantTable = constantTable;
    }
}
