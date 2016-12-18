package type;

import ast.Location;

/**
 * Created by sulvto on 16-12-18.
 */
public class UnionTypeRef extends TypeRef {
    private String name;

    public UnionTypeRef(String name) {
        super(null);
        this.name = name;
    }

    public UnionTypeRef(Location location, String name) {
        super(location);
        this.name = name;
    }

    @Override
    public String toString() {
        return "union " + name;
    }
}
