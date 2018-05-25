package base;

import java.util.Arrays;

/**
 * Created by sulvto on 17-10-29.
 */
public class StructSpace {
    public final Object[] fields;

    public StructSpace(int nFields) {
        this.fields = new Object[nFields];
    }

    @Override
    public String toString() {
        return Arrays.toString(fields);
    }

}
