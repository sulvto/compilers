package ast;

import entity.Constant;
import entity.DefinedFunction;
import entity.DefinedVariable;

import java.util.ArrayList;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Set;

/**
 * Created by sulvto on 16-12-13.
 */
public class Declarations {
    private Set<DefinedVariable> defvars = new LinkedHashSet<>();
    private Set<UndefinedVariable> vardecls = new LinkedHashSet<>();
    private Set<DefinedFunction> defuns = new LinkedHashSet<>();
    private Set<UndefinedFunction> funcdecls = new LinkedHashSet<>();
    private Set<Constant> constants = new LinkedHashSet<>();
    private Set<StructNode> defstructs = new LinkedHashSet<>();
    private Set<UnionNode> defunions = new LinkedHashSet<>();
    private Set<TypedefNode> typedefs = new LinkedHashSet<>();

    public Set<UndefinedVariable> vardecls() {
        return vardecls;
    }

    public Set<UndefinedFunction> funcdecls() {
        return funcdecls;
    }

    public void addDefvar(DefinedVariable definedVariable) {
        defvars.add(definedVariable);
    }

    public List<DefinedVariable> defvars() {
        return new ArrayList<>(defvars);
    }

    public void addDefun(DefinedFunction definedFunction) {
        defuns.add(definedFunction);
    }

    public List<DefinedFunction> defuns() {
        return new ArrayList<>(defuns);
    }

    public void addConstant(Constant constant) {
        constants.add(constant);
    }

    public List<Constant> constants() {
        return new ArrayList<>(constants);
    }

    public void addDefstruct(StructNode structNode) {
        defstructs.add(structNode);
    }

    public List<StructNode> defstructs() {
        return new ArrayList<>(defstructs);
    }

    public void addDefunion(UnionNode unionNode) {
        defunions.add(unionNode);
    }

    public List<UnionNode> defunions() {
        return new ArrayList<>(defunions);
    }

    public void addTypedef(TypedefNode typedefNode) {
        typedefs.add(typedefNode);
    }

    public List<TypedefNode> typedefs() {
        return new ArrayList<>(typedefs);
    }
}
