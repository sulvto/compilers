package interpreters;

import entity.DefinedVariable;
import entity.Entity;
import entity.Variable;

import java.util.HashMap;
import java.util.Map;

/**
 * Created by sulvto on 17-1-8.
 */
public class LocalScope extends Scope {
    private Scope parent;
    private Map<String, DefinedVariable> variables;

    public LocalScope(Scope parent) {
        this.parent = parent;
        variables = new HashMap<>();
    }

    @Override
    public boolean isToplevel() {
        return false;
    }

    @Override
    public ToplevelScope toplevel() {
        return parent.toplevel();
    }

    @Override
    public Scope parent() {
        return parent;
    }

    public void defineVariable(DefinedVariable variable) {
        String name = variable.getName();
        if (variables.containsKey(name)) {
            // TODO error;
        } else {
            variables.put(name, variable);
        }
    }

    @Override
    public Entity getEntity(String name) {
        DefinedVariable variable = variables.get(name);
        if (variable != null) {
            return variable;
        } else {
            return parent.getEntity(name);
        }
    }

}
