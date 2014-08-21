using JSON
using Glob

raw_ec_records = [JSON.parsefile(a) for a in ARGS]

# 3x3 arrays of counts, 2 symbols, 4 levels
ec_stats = zeros(3, 3, 2, 4)

for frames in raw_ec_records
    for frame in frames
        key_levels = ["mvf-l1" => 1, "mvf-l2" => 2, "mvf-l3" => 3, "mvf-l4" => 4]
        for key in keys(key_levels)
            if haskey(frame, key)
                for r in frame[key]
                    sym = r[1] + 1
                    prec_mvs = r[3] + r[4] + 1
                    same_mvs = r[5] + r[6] + 1
                    ec_stats[prec_mvs, same_mvs, sym, key_levels[key]] += 1
                end
            end
        end
    end
end

for level in 1:4
    totals = convert(Array{Int64,2}, ec_stats[:,:,1,level] .+ ec_stats[:,:,2,level])
    probs = convert(Array{Int64,2}, round((ec_stats[:,:,1,level] ./ totals) * 32768))
    println("LEVEL $level:")
    println("Probabilties:")
    println(probs)
    println("Totals:")
    println(totals)
    println()
end
