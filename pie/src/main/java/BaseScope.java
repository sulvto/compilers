import java.util.LinkedHashMap;
import java.util.Map;

/**
 * Created by sulvto on 17-10-27.
 */
public abstract class BaseScope implements Scope {
    Scope enclosingScope;
    Map<String, Symbol> symbols = new LinkedHashMap<>();

    public BaseScope(Scope parent) {
        this.enclosingScope = parent;
    }

    @Override
    public Symbol resolve(String name) {
        Symbol s = symbols.get(name);
        if (s != null) return s;
        if (getParentScope() != null) return getParentScope().resolve(name);
        return null;
    }

    @Override
    public void define(Symbol symbol) {
        symbols.put(symbol.name, symbol);
        symbol.scope = this;
    }

    public Scope getParentScope() {
        return getEnclosingScope();
    }

    @Override
    public Scope getEnclosingScope() {
        return enclosingScope;
    }

    @Override
    public String toString() {
        return symbols.keySet().toString();
    }

}
