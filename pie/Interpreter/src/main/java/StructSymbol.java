import java.util.LinkedHashMap;
import java.util.Map;

/**
 * Created by sulvto on 17-10-27.
 */
public class StructSymbol extends ScopedSymbol implements Scope {
    Map<String, Symbol> fields = new LinkedHashMap<>();

    public StructSymbol(String name, Scope parent) {
        super(name, parent);
    }

    public Symbol resolveMember(String name) {
        return fields.get(name);
    }

    @Override
    public Map<String, Symbol> getMembers() {
        return fields;
    }

    @Override
    public String toString() {
        return "struct " + name + ":{" + fields.keySet().toString() + "}";
    }
}
