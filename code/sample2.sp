import people for People
fun fn(){
    var p = People.new("xiaoming","male",20.0)
    p.sayHi()
}

class Family < People{
    var father
    var mother
    var child
    var age = 123
    new(f,m,c,a){
        father = f
        mother = m
        child = c
        age = a
        super("wbf","male",age)
    }
}

var f = Family.new("wbf","ls","shine",123)
f.sayHi()

fn()