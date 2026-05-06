#include "userprofilerepositorydb.h"
#include "modules/userprofile/domain/entities/userprofile.h"
#include "modules/userprofile/infrastructure/serializers/userprofileserializer.h"

#include <dbtoolkit/dbrepository.h>
#include <dbtoolkit/dbstorage.h>
#include <dbtoolkit/query/createtable.h>
#include <dbtoolkit/query/column.h>
#include <dbtoolkit/query/where.h>

UserProfileRepositoryDb::UserProfileRepositoryDb(DbStorage &storage)
    : m_repo(std::make_unique<DbRepository>(
          UserProfileSerializer::table,
          UserProfileSerializer::user_id_key,
          QStringList{
              UserProfileSerializer::user_id_key,
              UserProfileSerializer::language_key,
              UserProfileSerializer::timezone_key,
              UserProfileSerializer::sex_key,
              UserProfileSerializer::sessions_key,
              UserProfileSerializer::experience_key,
              UserProfileSerializer::goal_key,
              UserProfileSerializer::bodyweight_key,
              UserProfileSerializer::unit_system_key},
          storage, nullptr))
{
}

UserProfileRepositoryDb::~UserProfileRepositoryDb() = default;

bool UserProfileRepositoryDb::createTable()
{
    CreateTable table(UserProfileSerializer::table);
    table.ifNotExists()
        .column(Column(UserProfileSerializer::user_id_key).integer().primaryKey().notNull())
        .column(Column(UserProfileSerializer::language_key).text().notNull().defaultValue(QStringLiteral("en")))
        .column(Column(UserProfileSerializer::timezone_key).text().notNull().defaultValue(QStringLiteral("UTC")))
        .column(Column(UserProfileSerializer::sex_key).text().notNull().defaultValue(QStringLiteral("other")))
        .column(Column(UserProfileSerializer::sessions_key).integer().notNull().defaultValue(3))
        .column(Column(UserProfileSerializer::experience_key).text().notNull().defaultValue(QStringLiteral("beginner")))
        .column(Column(UserProfileSerializer::goal_key).text().notNull().defaultValue(QStringLiteral("general_fitness")))
        .column(Column(UserProfileSerializer::bodyweight_key).real())
        .column(Column(UserProfileSerializer::unit_system_key).text().notNull().defaultValue(QStringLiteral("metric")));

    return m_repo->createTable(table);
}

std::optional<UserProfile> UserProfileRepositoryDb::find() const
{
    auto rows = m_repo->select();
    if (rows.isEmpty())
        return std::nullopt;

    return UserProfileSerializer::fromVariant(rows.first());
}

void UserProfileRepositoryDb::save(const UserProfile &profile)
{
    m_repo->upsert(UserProfileSerializer::toVariant(profile));
}
