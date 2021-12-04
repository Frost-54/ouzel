// Ouzel by Elviss Strazdins

#ifndef OUZEL_ASSETS_CUELOADER_HPP
#define OUZEL_ASSETS_CUELOADER_HPP

#include "Loader.hpp"
#include "Bundle.hpp"
#include "../audio/Cue.hpp"
#include "../formats/Json.hpp"

namespace ouzel::assets
{
    class CueLoader final: public Loader
    {
    public:
        explicit CueLoader() noexcept: Loader{Asset::Type::cue} {}

        bool loadAsset(Cache&,
                       Bundle& bundle,
                       const std::string& name,
                       const std::vector<std::byte>& data,
                       const Asset::Options&) override
        {
            audio::SourceDefinition sourceDefinition;
            const auto d = json::parse(data);

            if (d.hasMember("source"))
                sourceDefinition = parseSourceDefinition(d["source"], bundle);

            auto cue = std::make_unique<audio::Cue>(sourceDefinition);

            bundle.setCue(name, std::move(cue));

            return true;
        }

    private:
        static auto getSourceType(const std::string& valueType)
        {
            if (valueType == "Parallel") return audio::SourceDefinition::Type::parallel;
            else if (valueType == "Random") return audio::SourceDefinition::Type::random;
            else if (valueType == "Sequence") return audio::SourceDefinition::Type::sequence;
            else if (valueType == "Oscillator") return audio::SourceDefinition::Type::oscillator;
            else if (valueType == "Silence") return audio::SourceDefinition::Type::silence;
            else if (valueType == "WavePlayer") return audio::SourceDefinition::Type::wavePlayer;
            else throw std::runtime_error("Invalid source type " + valueType);
        }

        static auto getOscillatorType(const std::string& oscillatorType)
        {
            if (oscillatorType == "Sine") return audio::Oscillator::Type::sine;
            else if (oscillatorType == "Square") return audio::Oscillator::Type::square;
            else if (oscillatorType == "Sawtooth") return audio::Oscillator::Type::sawtooth;
            else if (oscillatorType == "Triangle") return audio::Oscillator::Type::triangle;
            else throw std::runtime_error("Invalid oscillator type " + oscillatorType);
        }

        static auto getEffectType(const std::string& effectType)
        {
            if (effectType == "Delay") return audio::EffectDefinition::Type::delay;
            else if (effectType == "Gain") return audio::EffectDefinition::Type::gain;
            else if (effectType == "PitchScale") return audio::EffectDefinition::Type::pitchScale;
            else if (effectType == "PitchShift") return audio::EffectDefinition::Type::pitchShift;
            else if (effectType == "Reverb") return audio::EffectDefinition::Type::reverb;
            else if (effectType == "LowPass") return audio::EffectDefinition::Type::lowPass;
            else if (effectType == "HighPass") return audio::EffectDefinition::Type::highPass;
            else
                throw std::runtime_error("Invalid effect type " + effectType);
        }

        static audio::SourceDefinition parseSourceDefinition(const json::Value& value, Bundle& bundle)
        {
            audio::SourceDefinition sourceDefinition;

            sourceDefinition.type = getSourceType(value["type"].as<std::string>());

            if (sourceDefinition.type == audio::SourceDefinition::Type::oscillator)
            {
                sourceDefinition.oscillatorType = getOscillatorType(value["oscillatorType"].as<std::string>());

                if (value.hasMember("frequency"))
                    sourceDefinition.length = value["frequency"].as<float>();

                if (value.hasMember("amplitude"))
                    sourceDefinition.length = value["amplitude"].as<float>();

                if (value.hasMember("length"))
                    sourceDefinition.length = value["length"].as<float>();
            }
            else if (sourceDefinition.type == audio::SourceDefinition::Type::silence)
            {
                if (value.hasMember("length"))
                    sourceDefinition.length = value["length"].as<float>();
            }
            else if (sourceDefinition.type == audio::SourceDefinition::Type::wavePlayer)
            {
                if (value.hasMember("source"))
                    sourceDefinition.sound = bundle.getSound(value["source"].as<std::string>());
            }

            if (value.hasMember("effects"))
                for (const auto& effectValue : value["effects"])
                {
                    audio::EffectDefinition effectDefinition;
                    effectDefinition.type = getEffectType(effectValue["type"].as<std::string>());

                    if (effectValue.hasMember("delay")) effectDefinition.delay = effectValue["delay"].as<float>();
                    if (effectValue.hasMember("gain")) effectDefinition.gain = effectValue["gain"].as<float>();
                    if (effectValue.hasMember("scale")) effectDefinition.scale = effectValue["scale"].as<float>();
                    if (effectValue.hasMember("shift")) effectDefinition.shift = effectValue["shift"].as<float>();
                    if (effectValue.hasMember("decay")) effectDefinition.decay = effectValue["decay"].as<float>();

                    sourceDefinition.effectDefinitions.push_back(effectDefinition);
                }

            if (value.hasMember("sources"))
                for (const auto& sourceValue : value["sources"])
                    sourceDefinition.sourceDefinitions.push_back(parseSourceDefinition(sourceValue, bundle));

            return sourceDefinition;
        }
    };
}

#endif // OUZEL_ASSETS_CUELOADER_HPP
