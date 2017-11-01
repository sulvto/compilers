package ast;

import entity.DefinedFunction;
import entity.DefinedVariable;

import java.util.List;

/**
 * ast root
 * Created by sulvto on 16-12-8.
 */
public class AST extends Node {

    private Declarations declarations;

    public AST( Declarations declarations) {
        super(null);
        this.declarations = declarations;
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
}
