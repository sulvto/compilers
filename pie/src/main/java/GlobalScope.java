/**
 * Created by sulvto on 17-10-27.
 */
public class GlobalScope extends BaseScope {

    public GlobalScope() {
        super(null);
    }

    @Override
    public String getScopeName() {
        return "global";
    }
}
