#include "GameObject.h"

//Register_Class(GameObject);

GameObject::GameObject(const char *name, int o_type) : cOwnedObject(name)
{
	size = 0;
	type = o_type;
}

GameObject::GameObject(const GameObject& other) : cOwnedObject(other.getName())
{
	operator=(other);
}

GameObject::~GameObject()
{

}

//The following functions do not form part of a GameObject itself, but are required for the cOwnedObject type from which this object inherits.

GameObject *GameObject::dup() const
{
	return new GameObject(*this);
}

GameObject& GameObject::operator=(const GameObject& other)
{
	if (&other==this)
		return *this;
	cOwnedObject::operator=(other);


	size = other.size;
	type = other.type;

	return *this;
}

std::string GameObject::info()
{
	std::stringstream out;
	out << "GameObject: size = " << size << ", type = " << type;
	return out.str();
}

//This is the real meat of the packet. The getter and setter methods for the different attributes.
int64_t GameObject::getSize()
{
	return size;
}

void GameObject::setSize(int64_t o_size)
{
	size = o_size;
}

int GameObject::getType()
{
	return type;
}

void GameObject::setType(int o_type)
{
	type = o_type;
}

void GameObject::setObjectName(char *o_Name)
{
	strcpy(objectName, o_Name);
}

char * GameObject::getObjectName()
{
	return objectName;
}
