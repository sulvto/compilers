package entity;

import java.util.HashMap;
import java.util.Map;

/**
 * Created by sulvto on 17-11-1.
 */
public class LocalScope extends Scope {
    protected Scope parent;
    // local variable
    protected Map<String, DefinedVariable> variables;

    public LocalScope(Scope parent) {
        this.parent = parent;
        this.variables = new HashMap<>();
    }

    public void defineVariable(DefinedVariable variable) {
        variables.put(variable.getName(), variable);
    }

    public Entity get(String name) {
        DefinedVariable variable = variables.get(name);
        if (variable == null) {
            parent.get(name);
        }
        return variable;
    }

    public boolean isDefinedLocally(String name) {
        return variables.containsKey(name);
    }
}
