
/**
 * Created by sulvto on 17-10-27.
 */
public interface Scope {
    public String getScopeName();

    public Scope getEnclosingScope();

    public void define(Symbol symbol);

    public Symbol resolve(String name);
}
