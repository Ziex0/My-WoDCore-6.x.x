/*
 * Copyright (C) 2008-2015 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"

enum Texts
{
    SAY_AGGRO       = 0,
    SAY_EARTHQUAKE  = 1,
    SAY_OVERRUN     = 2,
    SAY_SLAY        = 3,
    SAY_DEATH       = 4
};

enum Spells
{
    SPELL_EARTHQUAKE        = 153616,
    SPELL_SUNDER_ARMOR      = 153726,
    SPELL_CHAIN_LIGHTNING   = 153764,
    SPELL_OVERRUN           = 154221,
    SPELL_ENRAGE            = 157173,
    SPELL_MARK_DEATH        = 153234,
    SPELL_AURA_DEATH        = 153616
};

enum Events
{
    EVENT_ENRAGE    = 1,
    EVENT_ARMOR     = 2,
    EVENT_CHAIN     = 3,
    EVENT_QUAKE     = 4,
    EVENT_OVERRUN   = 5
};

class boss_xin_the_weaponmaster : public CreatureScript
{
    public:
        boss_xin_the_weaponmaster() : CreatureScript("boss_xin_the_weaponmaster") { }

        struct boss_xin_the_weaponmasterAI : public ScriptedAI
        {
            boss_xin_the_weaponmasterAI(Creature* creature) : ScriptedAI(creature)
            {
                Initialize();
            }

            void Initialize()
            {
                _inEnrage = false;
            }

            void Reset() override
            {
                _events.Reset();
                _events.ScheduleEvent(EVENT_ENRAGE, 0);
                _events.ScheduleEvent(EVENT_ARMOR, urand(5000, 13000));
                _events.ScheduleEvent(EVENT_CHAIN, urand(10000, 30000));
                _events.ScheduleEvent(EVENT_QUAKE, urand(25000, 35000));
                _events.ScheduleEvent(EVENT_OVERRUN, urand(30000, 45000));
                Initialize();
            }

            void KilledUnit(Unit* victim) override
            {
                victim->CastSpell(victim, SPELL_MARK_DEATH, 0);

                if (urand(0, 4))
                    return;

                Talk(SAY_SLAY);
            }

            void JustDied(Unit* /*killer*/) override
            {
                Talk(SAY_DEATH);
            }

            void EnterCombat(Unit* /*who*/) override
            {
                Talk(SAY_AGGRO);
            }

            void MoveInLineOfSight(Unit* who) override

            {
                if (who && who->GetTypeId() == TYPEID_PLAYER && me->IsValidAttackTarget(who))
                    if (who->HasAura(SPELL_MARK_DEATH))
                        who->CastSpell(who, SPELL_AURA_DEATH, 1);
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                _events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = _events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_ENRAGE:
                            if (!HealthAbovePct(20))
                            {
                                DoCast(me, SPELL_ENRAGE);
                                _events.ScheduleEvent(EVENT_ENRAGE, 6000);
                                _inEnrage = true;
                            }
                            break;
                        case EVENT_OVERRUN:
                            Talk(SAY_OVERRUN);
                            DoCastVictim(SPELL_OVERRUN);
                            _events.ScheduleEvent(EVENT_OVERRUN, urand(25000, 40000));
                            break;
                        case EVENT_QUAKE:
                            if (urand(0, 1))
                                return;

                            Talk(SAY_EARTHQUAKE);

                            //remove enrage before casting earthquake because enrage + earthquake = 16000dmg over 8sec and all dead
                            if (_inEnrage)
                                me->RemoveAurasDueToSpell(SPELL_ENRAGE);

                            DoCast(me, SPELL_EARTHQUAKE);
                            _events.ScheduleEvent(EVENT_QUAKE, urand(30000, 55000));
                            break;
                        case EVENT_CHAIN:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 0.0f, true))
                                DoCast(target, SPELL_CHAIN_LIGHTNING);
                            _events.ScheduleEvent(EVENT_CHAIN, urand(7000, 27000));
                            break;
                        case EVENT_ARMOR:
                            DoCastVictim(SPELL_SUNDER_ARMOR);
                            _events.ScheduleEvent(EVENT_ARMOR, urand(10000, 25000));
                            break;
                        default:
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }

            private:
                EventMap _events;
                bool _inEnrage;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new boss_xin_the_weaponmasterAI(creature);
        }
};

void AddSC_boss_xin_the_weaponmaster()
{
    new boss_xin_the_weaponmaster();
}
