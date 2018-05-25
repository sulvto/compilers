import java.util.HashMap;
import java.util.Map;

/**
 * Created by sulvto on 17-10-27.
 */
public class MemorySpace {
    String name;
    private Map<String, Object> members = new HashMap<>();

    public MemorySpace(String name) {
        this.name = name;
    }

    public void put(String id, Object value) {
        members.put(id, value);
    }

    public Object get(String id) {
        return members.get(id);
    }

    @Override
    public String toString() {
        return name + ":" + members;
    }
}
