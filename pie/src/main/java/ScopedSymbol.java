import java.util.Map;

/**
 * Created by sulvto on 17-10-27.
 */
public abstract class ScopedSymbol extends Symbol implements Scope {
    Scope enclosingScope;

    public ScopedSymbol(String name, Scope enclosingScope) {
        super(name);
        this.enclosingScope = enclosingScope;
    }

    @Override
    public Symbol resolve(String name) {
        Symbol s = getMembers().get(name);
        if (s != null) return s;
        if (getParentScope() != null) return getParentScope().resolve(name);
        return null;
    }

    @Override
    public void define(Symbol symbol) {
        getMembers().put(symbol.name, symbol);
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
    public String getScopeName() {
        return name;
    }

    public abstract Map<String, Symbol> getMembers();
}
