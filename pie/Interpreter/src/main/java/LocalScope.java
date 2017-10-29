/**
 * Created by sulvto on 17-10-28.
 */
public class LocalScope extends BaseScope {
    public LocalScope(Scope parent) {
        super(parent);
    }

    @Override
    public String getScopeName() {
        return "local";
    }

}
