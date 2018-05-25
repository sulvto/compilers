package assembler;

/**
 * Created by sulvto on 17-10-29.
 */
public class FunctionSymbol {
    String name;
    int nargs;   // how many arguments are there
    int nlocals; // how many locals are there
    int address;

    public FunctionSymbol(String name) {
        this.name = name;
    }

    public FunctionSymbol(String name, int nargs, int nlocals, int address) {
        this.name = name;
        this.nargs = nargs;
        this.nlocals = nlocals;
        this.address = address;
    }

    public int getNargs() {
        return nargs;
    }

    public int getNlocals() {
        return nlocals;
    }

    public int getAddress() {
        return address;
    }

    @Override
    public int hashCode() {
        return name.hashCode();
    }

    @Override
    public boolean equals(Object obj) {
        return obj instanceof FunctionSymbol && name.equals(((FunctionSymbol) obj).name);
    }

    @Override
    public String toString() {
        return "FunctionSymbol{" +
                "name='" + name + "'" +
                ", nargs=" + nargs +
                ", nlocals=" + nlocals +
                ", address=" + address + "}";
    }
}
