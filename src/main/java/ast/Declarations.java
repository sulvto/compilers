package ast;

import entity.Constant;
import entity.DefinedFunction;
import entity.DefinedVariable;

import java.util.LinkedHashSet;
import java.util.Set;

/**
 * Created by sulvto on 16-12-13.
 */
public class Declarations {
    private Set<DefinedVariable> defvars = new LinkedHashSet<>();
    private Set<DefinedFunction> defuns = new LinkedHashSet<>();
    private Set<Constant> constants = new LinkedHashSet<>();
    private Set<StructNode> defstructs = new LinkedHashSet<>();
    private Set<UnionNode> defunions = new LinkedHashSet<>();
    private Set<TypedefNode> typedefs = new LinkedHashSet<>();


    public void addDefvar(DefinedVariable definedVariable) {
        defvars.add(definedVariable);
    }

    public void addDefun(DefinedFunction definedFunction) {
        defuns.add(definedFunction);
    }

    public void addConstant(Constant constant) {
        constants.add(constant);
    }

    public void addDefstruct(StructNode structNode) {
        defstructs.add(structNode);
    }

    public void addDefunion(UnionNode unionNode) {
        defunions.add(unionNode);
    }

    public void addTypedef(TypedefNode typedefNode) {
        typedefs.add(typedefNode);
    }
}
