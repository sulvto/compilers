/**
 * Created by sulvto on 17-10-27.
 */
public interface Scope {
    String getScopeName();

    Scope getEnclosingScope();

    void define(Symbol symbol);

    Symbol resolve(String name);
}
