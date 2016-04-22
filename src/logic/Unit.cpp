#include "Unit.hpp"

using namespace Tribalia::Logic;


Unit::Unit(int oid, int tid, const char* name)
    : AttackableObject(oid, tid, name)
    {
        this->AddProperty("speed", 1.0f);
        this->AddProperty("armatureMaterial", 0.0f);
        this->AddProperty("piercingAttack", 0.01f);
    }

Unit::Unit(int oid, int tid, const char* name,
    float x, float y, float z)
    : AttackableObject(oid, tid, name, x, y, z)
    {
        this->AddProperty("speed", 1.0f);
        this->AddProperty("armatureMaterial", 0.0f);
        this->AddProperty("piercingAttack", 0.01f);
    }

Unit::Unit(int oid, int tid, const char* name,
    float x, float y, float z, int maxHP,
    float baseAtk, float baseArmor)
    : AttackableObject(oid, tid, name, x, y, z, maxHP, baseAtk, baseArmor)
    {
        this->AddProperty("speed", 1.0f);
        this->AddProperty("armatureMaterial", 0.0f);
        this->AddProperty("piercingAttack", 0.01f);
    }

Unit::Unit(int oid, int tid, const char* name,
    float x, float y, float z, int maxHP,
    float baseAtk, float baseArmor,
    float speed, float armatureMaterial,
    float piercingAttack)
    : AttackableObject(oid, tid, name, x, y, z, maxHP, baseAtk, baseArmor)
    {
        this->AddProperty("speed", speed);
        this->AddProperty("armatureMaterial", armatureMaterial);
        this->AddProperty("piercingAttack", piercingAttack);
    }

float Unit::GetSpeed()
{
    return this->GetProperty<float>("speed");
}
void Unit::SetSpeed(float s)
{
    this->SetProperty("speed", s);
}

float Unit::GetArmatureMaterial()
{
    return this->GetProperty<float>("armatureMaterial");
}

void Unit::SetArmatureMaterial(float am)
{
    this->SetProperty("armatureMaterial", am);
}

float Unit::GetPiercingAttack()
{
    return this->GetProperty<float>("piercingAttack");
}

void Unit::SetPiercingAttack(float patk)
{
    this->SetProperty("piercingAttack", patk);
}
